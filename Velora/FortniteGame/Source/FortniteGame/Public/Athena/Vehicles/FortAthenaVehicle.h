#pragma once

#include "GameFramework/Actor.h"
#include "Delegates/Delegate.h"
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

struct FReplicatedAthenaVehiclePhysicsState
{
    FVector Translation;
    FQuat Rotation;
    FVector LinearVelocity;
    FVector AngularVelocity;
};

struct FVehicleDamageablePart
{
    int32 ConfigIndex;
    int32 BoneIndex;
    int32 ShapeIndex;
    float Health;
};

enum class EVehicleSeats : uint8
{
    Driver,
    Passenger1,
    Passenger2,
    Passenger3,
    Passenger4,
    Passenger5,
    MaxCount
};

struct FFrame;
struct FGameplayEffectContextHandle;
struct FGameplayTagContainer;
struct FReplicatedAthenaVehicleInputState;
class AController;
class AFortPlayerPawn;
class UPrimitiveComponent;

class AFortAthenaVehicle : public AActor
{
public:
    static UClass* StaticClass();

    static TMulticastDelegate<void, AActor*>& GetNotifyNetCullDistanceChange();

    static void InitializeServerImplementations();

    static const TCHAR* GetServerFunctionPath(const TCHAR* FunctionName);

    void ServerUpdatePhysicsParams_Implementation(const FReplicatedAthenaVehiclePhysicsState& InState);

    void ServerUpdateInputParams_Implementation(const FReplicatedAthenaVehicleInputState& InState);

    void WakeUp();

    AFortPlayerPawn* GetPawnAtSeat(int32 SeatIdx) const;

    bool ForceEnterSeat(AFortPlayerPawn* Pawn, EVehicleSeats Seat, bool bIgnoreDBNOCheck);

    float ModifyIncomingDamage(float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser);

    void AbsorbDamageIntoDamagablePart(float Damage, const FGameplayEffectContextHandle& EffectContext);

    void DestroyAllDamageableParts();

    void RepairAllDamageableParts(float NewHealth);

    void execDestroyAllDamageableParts(FFrame& Stack, void* const Z_Param__Result);

    void SetupToggleableShapeCollision();

    void OnRep_DamageableParts(const TArray<FVehicleDamageablePart>& PrevDamageableParts);

    TArray<FVehicleDamageablePart>& GetDamageableParts();

    UPrimitiveComponent* GetVehicleBulletCollisionComponent() const;
};
