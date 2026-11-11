#include "pch.h"
#include "Athena/FortAthenaSupplyDrop.h"
#include "Athena/GameStateComponents/FortGameStateComponent_BattleRoyaleGamePhaseLogic.h"
#include "Items/FortInventory.h"
#include "Items/FortPickup.h"
#include "Items/FortPickupAthena.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortWorldItemDefinition.h"
#include "Math/Rotator.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"
#include "Config.h"

UClass* AFortAthenaSupplyDrop::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaSupplyDrop"));

    return Class;
}

void AFortAthenaSupplyDrop::RegisterSpecialActor(const uint8* InSpecialActorTag, const uint8* MinimapIconBrush, const uint8* MinimapIconScale, const uint8* CompassIconBrush, const uint8* CompassIconScale, bool bShouldDrawCompassIcon)
{
    static auto RegisterSpecialActor = Anchor::Function<void (*)(AFortAthenaSupplyDrop*, const uint8*, const uint8*, const uint8*, const uint8*, const uint8*, bool)>(TEXT("AFortAthenaSupplyDrop::RegisterSpecialActor"))
        .String(TEXT("AFortAthenaMutator_Cobalt::SpawnAndRegisterSupplyDrop: Spawned and registered a supply drop"))
        .Start()
        .Find("lea rdx, [*+0x44]")
        .Find("call 0x*")
        .Target()
        .Get();

    RegisterSpecialActor(this, InSpecialActorTag, MinimapIconBrush, MinimapIconScale, CompassIconBrush, CompassIconScale, bShouldDrawCompassIcon);
}

UObject* AFortAthenaSupplyDrop::GetWaveSpawnSoundCue() const
{
    static auto WaveSpawnSoundCue = StaticClass()->FindPropertyByName(FName(TEXT("WaveSpawnSoundCue")));

    return *WaveSpawnSoundCue->ContainerPtrToValuePtr<UObject*>(this);
}

AFortPickup* AFortAthenaSupplyDrop::SpawnPickup(UFortWorldItemDefinition* ItemDefinition, int32 NumberToSpawn, AFortPawn* TriggeringPawn, const TGameValue<FVector>& GamePosition, const TGameValue<FVector>& GameDirection)
{
    DETOUR(AFortAthenaSupplyDrop, SpawnPickup, Anchor::Hook()
        .If(!DETOUR_ORIGINAL(AFortAthenaSupplyDrop, execSpawnPickup))
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:SpawnPickup")))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:SpawnPickup"))
        .Find("mov [*+0x20], *; movsd [*], xmm*; call 0x*")
        .Find("call 0x*")
        .AtCall());

    const FVector Position = GamePosition.Get();

    if (!ItemDefinition || NumberToSpawn <= 0)
        return nullptr;

    FFortItemEntry ItemEntry(ItemDefinition, NumberToSpawn, 0);

    static auto WeaponMelee = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponMelee")));
    static auto WeaponRanged = (EFortItemType)StaticEnum<EFortItemType>()->GetValueByName(FName(TEXT("EFortItemType::WeaponRanged")));

    if (ItemDefinition->GetItemType() == WeaponMelee || ItemDefinition->GetItemType() == WeaponRanged)
    {
        ItemEntry.SetDurability(ItemDefinition->GetMaxDurability(ItemEntry.GetLevel()));
        ItemEntry.SetStateValue(EFortItemEntryState::DurabilityInitialized, 1);
    }

    if (auto WeaponItemDefinition = Cast<UFortWeaponItemDefinition>(ItemDefinition))
    {
        if (auto WeaponStats = WeaponItemDefinition->GetWeaponStats<FFortBaseWeaponStats>())
            ItemEntry.SetLoadedAmmo(WeaponItemDefinition->GetBaseClipSizeWithMods(WeaponStats, ItemEntry.GetLevel()));
    }

    auto Pickup = AFortPickup::CreateFromData(GetWorld(), ItemEntry, Position, FRotator{ 0.0f, 0.0f, 0.0f }, nullptr, AFortPickupAthena::StaticClass(), this, EFortPickupSourceTypeFlag::Container);

    if (Pickup)
        Pickup->TossPickup(Position, nullptr, 0, true, true, EFortPickupSourceTypeFlag::Tossed, EFortPickupSpawnSource::Unset);

    return Pickup;
}

void AFortAthenaSupplyDrop::execSpawnPickup(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortAthenaSupplyDrop, execSpawnPickup, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:SpawnPickup")))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:SpawnPickup"))
        .Find("setnz *; add [*+0x20], *; call 0x*; and qword ptr [*], 0x0")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:SpawnPickup"))
        .Find("setnz *; add [*+0x20], *; call 0x*; mov rbx, [rsp+0x*]; mov [r*], r*")
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:SpawnPickup"))
        .Is("sub rsp, 0x28; call 0x*; cmp byte ptr [0x*], 0x0; jz 0x*; test rax, rax; jz 0x*; mov rcx, rax; call 0x*; xor eax, eax; add rsp, 0x28; ret")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:SpawnPickup"))
        .Find("xor ebp, ebp")
        .Find("setnz *; add r*, r*; mov [*+0x20], r*; call 0x*; mov [rsi], rbp; add rsp, *")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:SpawnPickup"))
        .Find("setnz *; add [*+0x20], *; call 0x*; mov [r*], rax")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:SpawnPickup")));

    UFortWorldItemDefinition* ItemDefinition = nullptr;
    int32 NumberToSpawn = 0;
    AFortPawn* TriggeringPawn = nullptr;
    TGameValue<FVector> Position;
    TGameValue<FVector> Direction;

    Stack.StepCompiledIn(&ItemDefinition);
    Stack.StepCompiledIn(&NumberToSpawn);
    Stack.StepCompiledIn(&TriggeringPawn);
    Stack.StepCompiledIn(&Position);
    Stack.StepCompiledIn(&Direction);
    Stack.Code += !!Stack.Code;

    *(AFortPickup**)Z_Param__Result = SpawnPickup(ItemDefinition, NumberToSpawn, TriggeringPawn, Position, Direction);
}

void AFortAthenaSupplyDrop::execGetSupplyDropSpawnData(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortAthenaSupplyDrop, execGetSupplyDropSpawnData, Anchor::Hook()
        .If(UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StaticClass() && UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StaticClass()->FindPropertyByName(FName(TEXT("SupplyDropSpawnDataList"))))
        .InExec(TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:GetSupplyDropSpawnData")));

    auto GamePhaseLogic = UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(this);

    if (!GamePhaseLogic)
        return Original(Stack, Z_Param__Result);

    float MinSpawnHeightTemp = 0.0f;
    float MaxSpawnHeightTemp = 0.0f;
    float& MinSpawnHeight = Stack.StepCompiledInRef<float>(&MinSpawnHeightTemp);
    float& MaxSpawnHeight = Stack.StepCompiledInRef<float>(&MaxSpawnHeightTemp);
    Stack.Code += !!Stack.Code;

    GamePhaseLogic->GetSupplyDropSpawnData(this, MinSpawnHeight, MaxSpawnHeight);

    if (Config::bLogReplication)
    {
        const FVector Location = GetActorLocation();

        UE_LOG(LogVelora, Log, TEXT("SupplyDrop %s at %.0f %.0f %.0f spawn height %.0f to %.0f"), *GetName(), Location.X, Location.Y, Location.Z, MinSpawnHeight, MaxSpawnHeight);
    }
}

void AFortAthenaSupplyDrop::execGetSupplyDropSpeedData(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortAthenaSupplyDrop, execGetSupplyDropSpeedData, Anchor::Hook()
        .If(UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StaticClass() && UFortGameStateComponent_BattleRoyaleGamePhaseLogic::StaticClass()->FindPropertyByName(FName(TEXT("SupplyDropSpawnDataList"))))
        .InExec(TEXT("/Script/FortniteGame.FortAthenaSupplyDrop:GetSupplyDropSpeedData")));

    auto GamePhaseLogic = UFortGameStateComponent_BattleRoyaleGamePhaseLogic::Get(this);

    if (!GamePhaseLogic)
        return Original(Stack, Z_Param__Result);

    float MinSpeedTemp = 0.0f;
    float MaxSpeedTemp = 0.0f;
    float& MinSpeed = Stack.StepCompiledInRef<float>(&MinSpeedTemp);
    float& MaxSpeed = Stack.StepCompiledInRef<float>(&MaxSpeedTemp);
    Stack.Code += !!Stack.Code;

    GamePhaseLogic->GetSupplyDropSpeedData(this, MinSpeed, MaxSpeed);

    if (Config::bLogReplication)
    {
        UE_LOG(LogVelora, Log, TEXT("SupplyDrop %s fall speed %.0f to %.0f"), *GetName(), MinSpeed, MaxSpeed);
    }
}
