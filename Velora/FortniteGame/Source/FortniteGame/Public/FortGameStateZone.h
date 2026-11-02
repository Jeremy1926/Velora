#pragma once

#include "FortGameState.h"
#include "Building/BuildingActor.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"

enum class EFortResourceType : uint8;
class AFortPlayerState;
class IAbilitySystemInterface;
class UBuildingStructuralSupportSystem;

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

    UBuildingStructuralSupportSystem* GetStructuralSupportSystem() const;
};

class UBuildingStructuralSupportSystem : public UObject
{
public:
    static UBuildingStructuralSupportSystem* GetCurrent(const UObject* WorldContextObject);

    bool IsGridInitialized() const;

    TMulticastDelegate<void>& GetOnGridInitialized();
};

class UFortCurieManager : public UActorComponent
{
public:
    void NotifyAllCurieManagerComponentsRegistered();

    void HandleBuildingGridInitialized();

    const TArray<UObject*>& GetCurieManagerComponents() const;
};
