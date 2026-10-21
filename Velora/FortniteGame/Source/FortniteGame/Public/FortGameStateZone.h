#pragma once

#include "FortGameState.h"
#include "Building/BuildingActor.h"

enum class EFortResourceType : uint8;
class AFortPlayerState;
class IAbilitySystemInterface;

struct FPlayerBuildableClassContainer
{
    TArray<UClass*> BuildingClasses;
};

struct FPlayerBuildableClassFilter
{
    EFortResourceType ResourceType;
    EFortBuildingType BuildingType;
    int32 BuildingLevel;
    UObject* EditModePatternData;
};

class AFortGameStateZone : public AFortGameState
{
public:
    static UClass* StaticClass();

    void GetPlayerBuildableClasses(TArray<UClass*>& OutBuildingClasses, const FPlayerBuildableClassFilter& Filter) const;

    const FPlayerBuildableClassContainer* GetPlayerBuildableClasses() const;

    const TArray<UClass*>& GetAllPlayerBuildableClasses() const;

    void OnAbilitySystemInterfaceReadyForPersistentGameplayEffects(IAbilitySystemInterface* AbilitySystemInterface);

    bool IsRespawningAllowed(AFortPlayerState* PlayerState) const;
};
