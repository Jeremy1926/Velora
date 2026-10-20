#pragma once

#include "HAL/Platform.h"
#include "Engine/NetSerialization.h"
#include "UObject/NameTypes.h"
#include "UObject/WeakObjectPtr.h"

class AActor;
class UPhysicalMaterial;
class UPrimitiveComponent;

enum class ELevelCollectionType
{
    DynamicSourceLevels,
    DynamicDuplicatedLevels,
    StaticLevels
};

enum ENetRole
{
    ROLE_None,
    ROLE_SimulatedProxy,
    ROLE_AutonomousProxy,
    ROLE_Authority,
    ROLE_MAX,
};

enum class ESpawnActorCollisionHandlingMethod : uint8
{
    Undefined,
    AlwaysSpawn,
    AdjustIfPossibleButAlwaysSpawn,
    AdjustIfPossibleButDontSpawnIfColliding,
    DontSpawnIfColliding,
};

enum ENetDormancy
{
    DORM_Never,
    DORM_Awake,
    DORM_DormantAll,
    DORM_DormantPartial,
    DORM_Initial,
    DORN_MAX,
};

enum EObjectTypeQuery
{
    ObjectTypeQuery1,
    ObjectTypeQuery2,
    ObjectTypeQuery3,
    ObjectTypeQuery4,
    ObjectTypeQuery5,
    ObjectTypeQuery6,
    ObjectTypeQuery7,
    ObjectTypeQuery8,
    ObjectTypeQuery9,
    ObjectTypeQuery10,
    ObjectTypeQuery11,
    ObjectTypeQuery12,
    ObjectTypeQuery13,
    ObjectTypeQuery14,
    ObjectTypeQuery15,
    ObjectTypeQuery16,
    ObjectTypeQuery17,
    ObjectTypeQuery18,
    ObjectTypeQuery19,
    ObjectTypeQuery20,
    ObjectTypeQuery21,
    ObjectTypeQuery22,
    ObjectTypeQuery23,
    ObjectTypeQuery24,
    ObjectTypeQuery25,
    ObjectTypeQuery26,
    ObjectTypeQuery27,
    ObjectTypeQuery28,
    ObjectTypeQuery29,
    ObjectTypeQuery30,
    ObjectTypeQuery31,
    ObjectTypeQuery32,
    ObjectTypeQuery_MAX,
};

struct FHitResult
{
    uint8 bBlockingHit : 1;
    uint8 bStartPenetrating : 1;
    int32 FaceIndex;
    float Time;
    float Distance;
    FVector_NetQuantize Location;
    FVector_NetQuantize ImpactPoint;
    FVector_NetQuantizeNormal Normal;
    FVector_NetQuantizeNormal ImpactNormal;
    FVector_NetQuantize TraceStart;
    FVector_NetQuantize TraceEnd;
    float PenetrationDepth;
    int32 Item;
    TWeakObjectPtr<UPhysicalMaterial> PhysMaterial;
    TWeakObjectPtr<AActor> Actor;
    TWeakObjectPtr<UPrimitiveComponent> Component;
    FName BoneName;
    FName MyBoneName;
};
