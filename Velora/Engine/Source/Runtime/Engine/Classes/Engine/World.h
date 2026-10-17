#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Engine/PendingNetGame.h"

class AActor;
class AGameModeBase;
class AGameNetworkManager;
class AGameStateBase;
class APawn;
class AWorldSettings;
class ULevel;
class UNetDriver;
struct FTransform;
struct FURL;

struct FActorSpawnParameters
{
    FActorSpawnParameters();

    FName Name;
    AActor* Template;
    AActor* Owner;
    APawn* Instigator;
    ULevel* OverrideLevel;
    ESpawnActorCollisionHandlingMethod SpawnCollisionHandlingOverride;

private:
    uint16 bRemoteOwned : 1;

public:
    uint16 bNoFail : 1;
    uint16 bDeferConstruction : 1;
    uint16 bAllowDuringConstructionScript : 1;
    EObjectFlags ObjectFlags;
};

struct FLevelCollection
{
public:
    void SetNetDriver(UNetDriver* const InNetDriver)
    {
        NetDriver = InNetDriver;
    }

private:
    ELevelCollectionType CollectionType;
    AGameStateBase* GameState;
    UNetDriver* NetDriver;
};

class UWorld final : public UObject, public FNetworkNotify
{
public:
    AActor* SpawnActor(UClass* InClass, FTransform const* UserTransformPtr, const FActorSpawnParameters& SpawnParameters = FActorSpawnParameters());

    AGameModeBase* GetAuthGameMode() const;

    AGameNetworkManager* GetNetworkManager() const;

    ULevel* GetCurrentLevelPendingVisibility() const;

    ULevel* GetCurrentLevelPendingInvisibility() const;

    float GetTimeSeconds() const;

    AWorldSettings* GetWorldSettings(bool bCheckStreamingPesistent = false, bool bChecked = true) const;

    UNetDriver* GetNetDriver() const;

    void SetNetDriver(UNetDriver* NewDriver);

    void SetNextSwitchCountdown(float InNextSwitchCountdown);

    FLevelCollection* FindCollectionByType(const ELevelCollectionType InType);

    bool Listen(FURL& InURL);
};
