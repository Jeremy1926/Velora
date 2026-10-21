#include "pch.h"
#include "FortGameStateZone.h"
#include "Building/BuildingSMActor.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AFortGameStateZone::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGameStateZone"));

    return Class;
}

void AFortGameStateZone::GetPlayerBuildableClasses(TArray<UClass*>& OutBuildingClasses, const FPlayerBuildableClassFilter& Filter) const
{
    OutBuildingClasses.Reset();

    const int32 FirstResourceType = (uint8)Filter.ResourceType < 4 ? (uint8)Filter.ResourceType : 0;
    const int32 LastResourceType = (uint8)Filter.ResourceType < 4 ? (uint8)Filter.ResourceType : 3;

    for (int32 ResourceType = FirstResourceType; ResourceType <= LastResourceType; ResourceType++)
    {
        auto& BuildingClasses = GetPlayerBuildableClasses()[ResourceType].BuildingClasses;

        if (Filter.BuildingLevel <= 0 && Filter.BuildingType == EFortBuildingType::None && !Filter.EditModePatternData)
        {
            OutBuildingClasses.Append(BuildingClasses);
            continue;
        }

        for (auto BuildingClass : BuildingClasses)
        {
            auto BuildingActor = (ABuildingSMActor*)(BuildingClass && BuildingClass->IsChildOf(ABuildingActor::StaticClass()) ? BuildingClass : nullptr)->GetDefaultObject();

            const bool bLevelMatches = Filter.BuildingLevel == -1 || Filter.BuildingLevel == BuildingActor->GetCurrentBuildingLevel();
            const bool bTypeMatches = Filter.BuildingType == EFortBuildingType::None || Filter.BuildingType == BuildingActor->GetBuildingType();
            const bool bPatternMatches = !Filter.EditModePatternData || Filter.EditModePatternData == BuildingActor->GetEditModePatternData();

            if (bLevelMatches && bTypeMatches && bPatternMatches)
                OutBuildingClasses.Add(BuildingClass);
        }
    }
}

const FPlayerBuildableClassContainer* AFortGameStateZone::GetPlayerBuildableClasses() const
{
    static auto PlayerBuildableClasses = GetClass()->FindPropertyByName(FName(TEXT("PlayerBuildableClasses")));

    return PlayerBuildableClasses->ContainerPtrToValuePtr<FPlayerBuildableClassContainer>(this);
}

const TArray<UClass*>& AFortGameStateZone::GetAllPlayerBuildableClasses() const
{
    static auto AllPlayerBuildableClasses = GetClass()->FindPropertyByName(FName(TEXT("AllPlayerBuildableClasses")));

    return *AllPlayerBuildableClasses->ContainerPtrToValuePtr<TArray<UClass*>>(this);
}

void AFortGameStateZone::OnAbilitySystemInterfaceReadyForPersistentGameplayEffects(IAbilitySystemInterface* AbilitySystemInterface)
{
    static auto OnAbilitySystemInterfaceReadyForPersistentGameplayEffects = Anchor::Function<void (*)(AFortGameStateZone*, IAbilitySystemInterface*)>(TEXT("AFortGameStateZone::OnAbilitySystemInterfaceReadyForPersistentGameplayEffects"))
        .String(TEXT("STAT_Fort_GameStateZoneApplyingPersistentEffects"))
        .Start()
        .Get();

    OnAbilitySystemInterfaceReadyForPersistentGameplayEffects(this, AbilitySystemInterface);
}

bool AFortGameStateZone::IsRespawningAllowed(AFortPlayerState* PlayerState) const
{
    static auto IsRespawningAllowed = Anchor::VirtualFunction<bool (*)(const AFortGameStateZone*, AFortPlayerState*)>(TEXT("AFortGameStateZone::IsRespawningAllowed"))
        .Exec(TEXT("/Script/FortniteGame.FortGameStateZone:IsRespawningAllowed"))
        .Find("call [rax+0x*]")
        .Get();

    return IsRespawningAllowed(this, PlayerState);
}
