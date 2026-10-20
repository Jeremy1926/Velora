#pragma once

#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "Templates/SubclassOf.h"

class AFortPlayerPawn;
class UGameplayEffect;
class USkeletalMeshComponent;
struct FScalableFloat;

class AFortAthenaSKPushCannon : public AFortAthenaVehicle
{
public:
    static UClass* StaticClass();

    void ServerLaunchActorInBarrel(const FVector& LaunchDir);

    void OnPreLaunchPawn(AFortPlayerPawn* Pawn, const FVector& LaunchDir);

    void OnLaunchPawn(AFortPlayerPawn* Pawn, const FVector& LaunchDir);

    void MultiCastPushCannonLaunchedPlayer();

    void MultiCastPushCannonLaunchedPlayer_Implementation();

    void PrelaunchDestroyBuildings(const TArray<FHitResult>& HitActors, AFortPlayerPawn* LaunchedPawn);

    void PawnLaunchedCosmetics(AFortPlayerPawn* FiredPlayer);

    USkeletalMeshComponent* GetSkeletalMesh() const;

    FScalableFloat& GetPlayerLaunchSpeed();

    FScalableFloat& GetPrelaunchSphereTraceEnabled();

    FScalableFloat& GetPrelaunchSphereTraceDistance();

    FScalableFloat& GetPrelaunchSphereTraceRadius();

    TSubclassOf<UGameplayEffect>& GetBuildingDestroyGE();
};
