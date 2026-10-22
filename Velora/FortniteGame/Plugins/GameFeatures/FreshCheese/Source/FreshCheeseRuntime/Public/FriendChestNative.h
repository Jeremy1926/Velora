#pragma once

#include "Building/BuildingContainer.h"
#include "Containers/Array.h"
#include "UObject/NameTypes.h"

struct FFrame;
struct FHitResult;
struct FScalableFloat;
class AActor;
class AGameModeBase;
class AFortPlayerPawnAthena;
class AFortPlayerStateAthena;
class UPrimitiveComponent;

class AFriendChestNative : public ABuildingContainer
{
public:
    static UClass* StaticClass();

    void execOnTriggerComponentBeginOverlap(FFrame& Stack, void* const Z_Param__Result);

    void execOnTriggerComponentEndOverlap(FFrame& Stack, void* const Z_Param__Result);

    void execRollInstancedLootForSquadMembers(FFrame& Stack, void* const Z_Param__Result);

    void OnTriggerComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    void OnTriggerComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void RollInstancedLootForSquadMembers(AGameModeBase* GameMode, uint8 SquadId);

    void GetAssociatedSquadMembers(bool bIncludeDead, bool bIncludeResurectable, bool bIncludeDisconnected, TArray<AFortPlayerStateAthena*>& OutSquadMembers) const;

    void PlayerAdded(AFortPlayerPawnAthena* Player);

    void PlayerRemoved(AFortPlayerPawnAthena* Player);

    static void TickActivationTimers();

    FScalableFloat& GetTrigerActivationDelayCurve();

    FName GetInstancedLootTierGroup() const;
};
