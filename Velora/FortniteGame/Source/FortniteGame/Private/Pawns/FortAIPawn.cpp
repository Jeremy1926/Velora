#include "pch.h"
#include "Pawns/FortAIPawn.h"
#include "Items/FortInventory.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortWorldItemDefinition.h"
#include "FortKismetLibrary.h"
#include "Engine/World.h"
#include "Math/Rotator.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"

static auto HandleDeathNotifications = []
{
    auto GameHandleDeathNotifications = Scanner::CallTo(Scanner::String(TEXT("Failed to find spawn group info on enemy pawn death for pawn %s. Associated Mission Name: %s. Group GUID is %s. Num active groups %i.")).Start().Get()).Start().Get();
    auto CallHandleDeathNotifications = std::format("call 0x{:x}", (uintptr_t)GameHandleDeathNotifications);
    auto DefaultObject = GameHandleDeathNotifications ? StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortAIPawn")) : nullptr;
    auto VTable = DefaultObject ? *(const void***)DefaultObject : nullptr;
    auto Destroyed = VTable ? Scanner::Slot(TEXT("AFortAIPawn::Destroyed"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Has(CallHandleDeathNotifications); }) : 0;
    auto HandleOutOfHealth = Scanner::IsSlot(Destroyed) ? Scanner::Slot(TEXT("AFortAIPawn::HandleOutOfHealth"), VTable, [&](const Scanner::FCode& Entry) { return Entry.Has(CallHandleDeathNotifications); }, Destroyed / (int32)sizeof(void*) + 1) : 0;

    return Scanner::IsSlot(HandleOutOfHealth) ? DetourCall(TEXT("AFortAIPawn::HandleDeathNotifications"), Scanner::At(VTable[HandleOutOfHealth / sizeof(void*)]).Find(CallHandleDeathNotifications).Get(), &AFortAIPawn::HandleDeathNotifications) : nullptr;
}();

static std::unordered_map<AFortAIPawn*, std::tuple<FWeakObjectPtr, float, FName>> LootDropData;

UClass* AFortAIPawn::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAIPawn"));

    return Class;
}

void AFortAIPawn::HandleDeathNotifications()
{
    DropLoot();

    (this->*::HandleDeathNotifications)();
}

void AFortAIPawn::SetLootDropData(float LootDropChance, FName WorldItemTierGroup)
{
    LootDropData.insert_or_assign(this, std::tuple{ FWeakObjectPtr(this), LootDropChance, WorldItemTierGroup });
}

void AFortAIPawn::DropLoot()
{
    auto Found = LootDropData.find(this);

    if (Found == LootDropData.end())
        return;

    auto [Pawn, LootDropChance, WorldItemTierGroup] = Found->second;

    LootDropData.erase(Found);

    if (!Pawn.HasSameIndexAndSerialNumber(FWeakObjectPtr(this)) || GetHasDroppedLoot() || WorldItemTierGroup == NAME_None || FMath::FRand() >= LootDropChance)
        return;

    SetHasDroppedLoot(true);

    TArray<FFortItemEntry> LootToDrop;

    if (!UFortKismetLibrary::PickLootDrops(&LootToDrop, WorldItemTierGroup, -1, -1))
        return;

    const FVector LootSpawnLocation = GetActorLocation();

    for (auto& ItemEntry : LootToDrop)
    {
        if (!Cast<UFortWorldItemDefinition>(ItemEntry.GetItemDefinition()))
            continue;

        if (auto Pickup = AFortPickup::CreateFromData(GetWorld(), ItemEntry, LootSpawnLocation, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass(), nullptr, EFortPickupSourceTypeFlag::AI))
            Pickup->TossPickup(LootSpawnLocation, nullptr, 0, true);
    }
}

bool AFortAIPawn::GetHasDroppedLoot() const
{
    static auto bHasDroppedLoot = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bHasDroppedLoot")));

    return bHasDroppedLoot->GetPropertyValue_InContainer(this);
}

void AFortAIPawn::SetHasDroppedLoot(bool bInHasDroppedLoot)
{
    static auto bHasDroppedLoot = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bHasDroppedLoot")));

    bHasDroppedLoot->SetPropertyValue_InContainer(this, bInHasDroppedLoot);
}
