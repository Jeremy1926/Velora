#pragma once

#include "Building/BuildingSMActor.h"

enum class EFortBuildingPersistentState : uint8;
struct FReservedRandomValues;
struct FFrame;
class UScriptStruct;

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
    static UScriptStruct* StaticStruct();

    FVector& GetTranslation();

    void SetRotation(const FQuat& InRotation);

    EDynamicFoundationEnabledState& GetEnabledState();
};

class ABuildingFoundation : public ABuildingSMActor
{
public:
    void PostUpdate(EFortBuildingPersistentState PersistentState, const FReservedRandomValues* ReservedRandomValues);

    void SetDynamicFoundationTransform(const FTransform& NewTransform);

    void execSetDynamicFoundationTransform(FFrame& Stack, void* const Z_Param__Result);

    void SetDynamicFoundationEnabled(bool bEnabled);

    void execSetDynamicFoundationEnabled(FFrame& Stack, void* const Z_Param__Result);

    void SelectAndStreamInLevel(const FReservedRandomValues* ReservedRandomValues);

    bool StreamOutMyBuilding();

    void OnRep_DynamicFoundationTransform();

    void OnRep_DynamicFoundationRepData();

    void OnRep_FoundationEnabledState();

    void OnRep_FoundationEnabled(bool bOldEnabled);

    bool TransformIsSet() const;

    EDynamicFoundationType GetDynamicFoundationType() const;

    FName GetLevelToStream() const;

    FTransform& GetDynamicFoundationTransform();
};
