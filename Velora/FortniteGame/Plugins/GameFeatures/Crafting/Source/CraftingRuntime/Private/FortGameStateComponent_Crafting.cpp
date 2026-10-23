#include "pch.h"
#include "FortGameStateComponent_Crafting.h"
#include "Engine/World.h"
#include "FortKismetLibrary.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTagContainer.h"
#include "Containers/Map.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* UFortGameStateComponent_Crafting::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CraftingRuntime.FortGameStateComponent_Crafting"));

    return Class;
}

UFortGameStateComponent_Crafting* UFortGameStateComponent_Crafting::Get(const UObject* WorldContextObject)
{
    auto World = WorldContextObject && StaticClass() ? WorldContextObject->GetWorld() : nullptr;
    auto GameState = World ? (AActor*)World->GetGameState() : nullptr;

    return GameState ? (UFortGameStateComponent_Crafting*)GameState->FindComponentByClass(StaticClass()) : nullptr;
}

void UFortGameStateComponent_Crafting::BuildCraftingFormulaAccelerationData()
{
    DETOUR(UFortGameStateComponent_Crafting, BuildCraftingFormulaAccelerationData, Anchor::Hook()
        .Pattern("48 8B 93 B0 00 00 00 48 8B C8 E8 ? ? ? ? 48 8B F8 4C 8D 35 ? ? ? ? 48 85 FF 74 ? 48 8B CB E8")
        .Call(2)
        .Optional()
        .Inline());

    Original();

    auto Owner = GetOwner();

    if (Owner && Owner->HasAuthority())
        UpdateCraftingResultsList();
}

void UFortGameStateComponent_Crafting::UpdateCraftingResultsList()
{
    static auto GetCraftingFormulaForRowName = Anchor::Function<void* (*)(UFortGameStateComponent_Crafting*, const FName*, bool)>(TEXT("UFortGameStateComponent_Crafting::GetCraftingFormulaForRowName"))
        .Pattern("48 8B 93 B0 00 00 00 48 8B C8 E8 ? ? ? ? 48 8B F8 4C 8D 35 ? ? ? ? 48 85 FF 74 ? 48 8B CB E8")
        .Call(2)
        .Find("xor r8d, r8d; mov rdx, *; mov rcx, *; call 0x*")
        .Call()
        .Get();

    static auto CraftingObjectToFormulaName = Anchor::Offset(TEXT("UFortGameStateComponent_Crafting::CraftingObjectToFormulaName"))
        .Pattern("48 8B 93 B0 00 00 00 48 8B C8 E8 ? ? ? ? 48 8B F8 4C 8D 35 ? ? ? ? 48 85 FF 74 ? 48 8B CB E8")
        .Call(2)
        .Find("lea *, [rcx+0x*]")
        .Disp();

    static auto ResultLootTierKey = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CraftingRuntime.CraftingFormula")))->FindPropertyByName(FName(TEXT("ResultLootTierKey")));

    if (CraftingObjectToFormulaName <= 0 || !ResultLootTierKey)
        return;

    auto& CraftingResultsList = GetCraftingResultsList();
    const int32 PreviousNum = CraftingResultsList.Num();

    for (auto& Pair : *(TMap<FGameplayTag, TArray<FName>>*)((uint8*)this + CraftingObjectToFormulaName))
    {
        for (int32 FormulaIndex = 0; FormulaIndex < Pair.Value.Num(); FormulaIndex++)
        {
            const FName FormulaRow = TGameLayout<FName>::Load((uint8*)Pair.Value.GetData() + FormulaIndex * TGameLayout<FName>::Size());
            auto CraftingFormula = GetCraftingFormulaForRowName(this, &FormulaRow, false);

            if (!CraftingFormula)
                continue;

            const FName LootTierKey = ResultLootTierKey->GetPropertyValue_InContainer<FName>(CraftingFormula);
            bool bAlreadyListed = false;

            for (auto& CraftingResult : CraftingResultsList)
                bAlreadyListed |= CraftingResult.ResultLootTierKey == LootTierKey;

            TArray<FFortItemEntry> LootToDrop;

            if (bAlreadyListed || !UFortKismetLibrary::PickLootDrops(&LootToDrop, LootTierKey, -1, -1))
                continue;

            FCraftingResult CraftingResult;
            CraftingResult.ResultLootTierKey = LootTierKey;

            for (auto& ItemEntry : LootToDrop)
                CraftingResult.Results.Add(FItemAndCount{ ItemEntry.GetCount(), ItemEntry.GetItemDefinition() });

            CraftingResultsList.Add(CraftingResult);
        }
    }

    if (CraftingResultsList.Num() != PreviousNum)
        OnRep_CraftingResultsList();
}

void UFortGameStateComponent_Crafting::OnRep_CraftingResultsList()
{
    static auto Function = (UFunction*)StaticFindObject(nullptr, nullptr, TEXT("/Script/CraftingRuntime.FortGameStateComponent_Crafting:OnRep_CraftingResultsList"));

    alignas(FFrame) uint8 Stack[sizeof(FFrame)] = {};

    (this->*Function->GetNativeFunc())(*(FFrame*)Stack, nullptr);
}

TArray<FCraftingResult>& UFortGameStateComponent_Crafting::GetCraftingResultsList()
{
    static auto CraftingResultsList = StaticClass()->FindPropertyByName(FName(TEXT("CraftingResultsList")));

    return *CraftingResultsList->ContainerPtrToValuePtr<TArray<FCraftingResult>>(this);
}
