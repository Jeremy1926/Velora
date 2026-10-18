#include "pch.h"
#include "FortGameMode.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

static int32_t OnPlayerCreatedBuildingSlot()
{
    static auto OnPlayerCreatedBuilding = []
    {
        auto VTable = *(const uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
        auto Index = 0;

        while (!Scanner::At(VTable[Index]).Is("mov r9, r8; mov r8, rdx; mov dl, *; jmp 0x*"))
            Index++;

        auto BuildingAction = std::format("call {:#x}", (uintptr_t)Scanner::At(VTable[Index]).Find("jmp 0x*").Target(TEXT("AFortGameMode::OnPlayerBuildingAction")));
        Index = 0;

        while (!Scanner::At(VTable[Index]).Start().Has(BuildingAction))
            Index++;

        return Scanner::Slot(TEXT("AFortGameMode::OnPlayerCreatedBuilding"), Index * (int32_t)sizeof(void*));
    }();

    return OnPlayerCreatedBuilding;
}

UClass* AFortGameMode::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameMode"));

    return Class;
}

void AFortGameMode::OnPlayerCreatedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor)
{
    auto VTable = *(uint8_t**)this;

    (*(void (**)(AFortGameMode*, AFortPlayerController*, ABuildingActor*))(VTable + OnPlayerCreatedBuildingSlot()))(this, PlayerController, BuildingActor);
}

void AFortGameMode::OnPlayerRepairedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor)
{
    auto VTable = *(uint8_t**)this;

    (*(void (**)(AFortGameMode*, AFortPlayerController*, ABuildingActor*))(VTable + OnPlayerCreatedBuildingSlot() + 3 * sizeof(void*)))(this, PlayerController, BuildingActor);
}

void AFortGameMode::OnPlayerEditedBuilding(AFortPlayerController* PlayerController, ABuildingActor* BuildingActor)
{
    auto VTable = *(uint8_t**)this;

    (*(void (**)(AFortGameMode*, AFortPlayerController*, ABuildingActor*))(VTable + OnPlayerCreatedBuildingSlot() + 4 * sizeof(void*)))(this, PlayerController, BuildingActor);
}

void AFortGameMode::FinishWorldInitialization()
{
    static auto FinishWorldInitialization = []
    {
        auto AthenaFinishWorldInitialization = Scanner::String(TEXT("Can't find a FortAthenaMapInfo placed in map.  Skipping warmup and aircraft phases.")).Start().Get(TEXT("AFortGameModeAthena::FinishWorldInitialization"));
        auto AthenaVTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
        auto Index = 0;

        while (AthenaVTable[Index] != AthenaFinishWorldInitialization)
            Index++;

        auto VTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameMode"));

        return (void (*)(AFortGameMode*))VTable[Index];
    }();

    FinishWorldInitialization(this);
}

void AFortGameMode::OnGivenMatchAssignment(const TMap<FString, FString>& MatchOptions)
{
    static auto OnGivenMatchAssignment = []
    {
        auto AthenaOnGivenMatchAssignment = Scanner::String(TEXT("match.option.warmupRequiredPlayerCount")).Start().Get(TEXT("AFortGameModeAthena::OnGivenMatchAssignment"));
        auto AthenaVTable = *(uint8_t***)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.Default__FortGameModeAthena"));
        auto Index = 0;

        while (AthenaVTable[Index] != AthenaOnGivenMatchAssignment)
            Index++;

        return Index * (int32_t)sizeof(void*);
    }();

    auto VTable = *(uint8_t**)this;

    (*(void (**)(AFortGameMode*, const TMap<FString, FString>&))(VTable + OnGivenMatchAssignment))(this, MatchOptions);
}

AFortGameSession* AFortGameMode::GetFortGameSession()
{
    static auto FortGameSession = GetClass()->FindPropertyByName(FName(TEXT("FortGameSession")));

    return *FortGameSession->ContainerPtrToValuePtr<AFortGameSession*>(this);
}
