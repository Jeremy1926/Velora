#pragma once

#include "Building/BuildingSMActor.h"

enum class EFortBuildingPersistentState : uint8;
struct FReservedRandomValues;

enum class EDynamicFoundationType : uint8
{
    Static,
    StartEnabled_Stationary,
    StartEnabled_Dynamic,
    StartDisabled,
    EDynamicFoundationType_MAX
};

enum class EDynamicFoundationEnabledState : uint8
{
    Unknown,
    Enabled,
    Disabled,
    EDynamicFoundationEnabledState_MAX
};

struct FDynamicBuildingFoundationRepData
{
    FVector Translation;
    FQuat Rotation;
    EDynamicFoundationEnabledState EnabledState;
};

class ABuildingFoundation : public ABuildingSMActor
{
public:
    void PostUpdate(EFortBuildingPersistentState PersistentState, const FReservedRandomValues* ReservedRandomValues);

    void SetDynamicFoundationTransform(const FTransform& NewTransform);

    void OnRep_DynamicFoundationTransform();

    void OnRep_DynamicFoundationRepData();

    bool TransformIsSet() const;

    EDynamicFoundationType GetDynamicFoundationType() const;

    FTransform& GetDynamicFoundationTransform();
};
