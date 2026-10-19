#pragma once

#include "Building/BuildingSMActor.h"

enum class EFortBuildingPersistentState : uint8;
struct FReservedRandomValues;

enum class EDynamicFoundationType : uint8
{
    Static,
    Dynamic_StartEnabled,
    EDynamicFoundationType_MAX
};

class ABuildingFoundation : public ABuildingSMActor
{
public:
    void PostUpdate(EFortBuildingPersistentState PersistentState, const FReservedRandomValues* ReservedRandomValues);

    void SetDynamicFoundationTransform(const FTransform& NewTransform);

    void OnRep_DynamicFoundationTransform();

    bool TransformIsSet() const;

    EDynamicFoundationType GetDynamicFoundationType() const;

    FTransform& GetDynamicFoundationTransform();
};
