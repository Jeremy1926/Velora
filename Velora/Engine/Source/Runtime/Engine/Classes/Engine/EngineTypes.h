#pragma once

#include "HAL/Platform.h"
#include "HAL/UnrealMemory.h"
#include "Engine/NetSerialization.h"
#include "UObject/NameTypes.h"
#include "UObject/WeakObjectPtr.h"

class AActor;
class UPrimitiveComponent;
class UScriptStruct;

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

namespace ECollisionEnabled
{
    enum Type
    {
        NoCollision,
        QueryOnly,
        PhysicsOnly,
        QueryAndPhysics
    };
}

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

enum ECollisionChannel
{
    ECC_WorldStatic,
    ECC_WorldDynamic,
    ECC_Pawn,
    ECC_Visibility,
    ECC_Camera,
    ECC_PhysicsBody,
    ECC_Vehicle,
    ECC_Destructible,
    ECC_EngineTraceChannel1,
    ECC_EngineTraceChannel2,
    ECC_EngineTraceChannel3,
    ECC_EngineTraceChannel4,
    ECC_EngineTraceChannel5,
    ECC_EngineTraceChannel6,
    ECC_GameTraceChannel1,
    ECC_GameTraceChannel2,
    ECC_GameTraceChannel3,
    ECC_GameTraceChannel4,
    ECC_GameTraceChannel5,
    ECC_GameTraceChannel6,
    ECC_GameTraceChannel7,
    ECC_GameTraceChannel8,
    ECC_GameTraceChannel9,
    ECC_GameTraceChannel10,
    ECC_GameTraceChannel11,
    ECC_GameTraceChannel12,
    ECC_GameTraceChannel13,
    ECC_GameTraceChannel14,
    ECC_GameTraceChannel15,
    ECC_GameTraceChannel16,
    ECC_GameTraceChannel17,
    ECC_GameTraceChannel18,
    ECC_OverlapAll_Deprecated,
    ECC_MAX,
};

enum class ETraceTypeQuery : uint8
{
    TraceTypeQuery1,
    TraceTypeQuery2,
    TraceTypeQuery3,
    TraceTypeQuery4,
    TraceTypeQuery5,
    TraceTypeQuery6,
    TraceTypeQuery7,
    TraceTypeQuery8,
    TraceTypeQuery9,
    TraceTypeQuery10,
    TraceTypeQuery11,
    TraceTypeQuery12,
    TraceTypeQuery13,
    TraceTypeQuery14,
    TraceTypeQuery15,
    TraceTypeQuery16,
    TraceTypeQuery17,
    TraceTypeQuery18,
    TraceTypeQuery19,
    TraceTypeQuery20,
    TraceTypeQuery21,
    TraceTypeQuery22,
    TraceTypeQuery23,
    TraceTypeQuery24,
    TraceTypeQuery25,
    TraceTypeQuery26,
    TraceTypeQuery27,
    TraceTypeQuery28,
    TraceTypeQuery29,
    TraceTypeQuery30,
    TraceTypeQuery31,
    TraceTypeQuery32,
    TraceTypeQuery_MAX,
};

enum class EAttachmentRule : uint8
{
    KeepRelative,
    KeepWorld,
    SnapToTarget,
};

enum class EDetachmentRule : uint8
{
    KeepRelative,
    KeepWorld,
};

struct FAttachmentTransformRules
{
    FAttachmentTransformRules(EAttachmentRule InLocationRule, EAttachmentRule InRotationRule, EAttachmentRule InScaleRule, bool bInWeldSimulatedBodies)
        : LocationRule(InLocationRule)
        , RotationRule(InRotationRule)
        , ScaleRule(InScaleRule)
        , bWeldSimulatedBodies(bInWeldSimulatedBodies)
    {
    }

    EAttachmentRule LocationRule;
    EAttachmentRule RotationRule;
    EAttachmentRule ScaleRule;
    bool bWeldSimulatedBodies;
};

struct FDetachmentTransformRules
{
    FDetachmentTransformRules(EDetachmentRule InLocationRule, EDetachmentRule InRotationRule, EDetachmentRule InScaleRule, bool bInCallModify)
        : LocationRule(InLocationRule)
        , RotationRule(InRotationRule)
        , ScaleRule(InScaleRule)
        , bCallModify(bInCallModify)
    {
    }

    EDetachmentRule LocationRule;
    EDetachmentRule RotationRule;
    EDetachmentRule ScaleRule;
    bool bCallModify;
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

struct alignas(8) FHitResult
{
    FHitResult();

    static UScriptStruct* StaticStruct();

    static int32 GetTypeSize();

    float& GetTime();

    FVector_NetQuantize& GetLocation();

    TWeakObjectPtr<AActor>& GetActor();

    const TWeakObjectPtr<UPrimitiveComponent>& GetComponent() const;

    uint8 GetElementIndex() const;

private:
    uint8 Data[0x100];
};

template<>
class TArray<FHitResult, FDefaultAllocator> : public TRuntimeSizedArray<FHitResult>
{
};
