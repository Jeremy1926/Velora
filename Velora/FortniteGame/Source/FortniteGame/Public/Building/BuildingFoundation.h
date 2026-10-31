#pragma once

#include "Building/BuildingSMActor.h"
#include "Math/Quat.h"
#include "Math/TransformVectorized.h"

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

    void SetTranslation(const FVector& InTranslation);

    void SetRotation(const FQuat& InRotation);

    EDynamicFoundationEnabledState& GetEnabledState();
};

class ABuildingFoundation : public ABuildingSMActor
{
public:
    void PostUpdate(EFortBuildingPersistentState PersistentState, const FReservedRandomValues* ReservedRandomValues);

    void SetDynamicFoundationTransform(const TGameValue<FTransform>& GameNewTransform);

    void execSetDynamicFoundationTransform(FFrame& Stack, void* const Z_Param__Result);

    void SetDynamicFoundationEnabled(bool bEnabled);

    void execSetDynamicFoundationEnabled(FFrame& Stack, void* const Z_Param__Result);

    void SelectAndStreamInLevel(const FReservedRandomValues* ReservedRandomValues);

    void OnRep_DynamicFoundationTransform();

    void OnRep_DynamicFoundationRepData();

    void OnRep_FoundationEnabledState();

    void OnRep_FoundationEnabled(bool bOldEnabled);

    bool TransformIsSet() const;

    EDynamicFoundationType GetDynamicFoundationType() const;

    FName GetLevelToStream() const;
};
