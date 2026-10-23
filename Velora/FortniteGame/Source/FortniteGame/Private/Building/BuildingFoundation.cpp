#include "pch.h"
#include "Building/BuildingFoundation.h"
#include "FortGameState.h"
#include "FortWorldManager.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UScriptStruct* FDynamicBuildingFoundationRepData::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.DynamicBuildingFoundationRepData"));

    return Struct;
}

FVector& FDynamicBuildingFoundationRepData::GetTranslation()
{
    static auto Translation = StaticStruct()->FindPropertyByName(FName(TEXT("Translation")));

    return *Translation->ContainerPtrToValuePtr<FVector>(this);
}

void FDynamicBuildingFoundationRepData::SetRotation(const FQuat& InRotation)
{
    static auto Rotation = StaticStruct()->FindPropertyByName(FName(TEXT("Rotation")));

    if (Rotation->GetSize() == sizeof(FQuat))
        *Rotation->ContainerPtrToValuePtr<FQuat>(this) = InRotation;
    else
        *Rotation->ContainerPtrToValuePtr<FRotator>(this) = InRotation.Rotator();
}

EDynamicFoundationEnabledState& FDynamicBuildingFoundationRepData::GetEnabledState()
{
    static auto EnabledState = StaticStruct()->FindPropertyByName(FName(TEXT("EnabledState")));

    return *EnabledState->ContainerPtrToValuePtr<EDynamicFoundationEnabledState>(this);
}

void ABuildingFoundation::SetDynamicFoundationTransform(const FTransform& NewTransform)
{
    DETOUR(ABuildingFoundation, SetDynamicFoundationTransform, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.BuildingFoundation:SetDynamicFoundationTransform"))
        .Find("mov [*+0x20], *; call 0x*")
        .Find("call 0x*")
        .IsTarget("ret*")
        .Optional()
        .AtCall());

    static auto Offset = Anchor::Offset(TEXT("ABuildingFoundation::TransformIsSet"))
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Log(TEXT("ABuildingFoundation::SelectAndStreamInLevel"))
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Last("test byte ptr [*+0x*], 0x*")
        .Or()
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Caller()
        .Log(TEXT("ABuildingFoundation::SelectAndStreamInLevel"))
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Last("test byte ptr [*+0x*], 0x*")
        .DispValue();

    static auto Mask = (uint8_t)Anchor::Offset(TEXT("ABuildingFoundation::TransformIsSet"))
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Last("test byte ptr [*+0x*], 0x*")
        .Or()
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Caller()
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Last("test byte ptr [*+0x*], 0x*")
        .Value();

    static auto DynamicFoundationRepData = GetClass()->FindPropertyByName(FName(TEXT("DynamicFoundationRepData")));

    FlushNetDormancy();

    GetDynamicFoundationTransform() = NewTransform;
    *((uint8_t*)this + Offset) |= Mask;

    if (!DynamicFoundationRepData)
    {
        OnRep_DynamicFoundationTransform();
        return;
    }

    auto& RepData = *DynamicFoundationRepData->ContainerPtrToValuePtr<FDynamicBuildingFoundationRepData>(this);
    RepData.GetTranslation() = NewTransform.GetTranslation();
    RepData.SetRotation(NewTransform.GetRotation());

    OnRep_DynamicFoundationRepData();
}

void ABuildingFoundation::execSetDynamicFoundationTransform(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(ABuildingFoundation, execSetDynamicFoundationTransform, Anchor::Hook()
        .StubExec(TEXT("/Script/FortniteGame.BuildingFoundation:SetDynamicFoundationTransform"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.BuildingFoundation:SetDynamicFoundationTransform")));

    FTransform NewTransform;

    Stack.StepCompiledIn(&NewTransform);
    Stack.Code += !!Stack.Code;

    SetDynamicFoundationTransform(NewTransform);
}

void ABuildingFoundation::PostUpdate(EFortBuildingPersistentState PersistentState, const FReservedRandomValues* ReservedRandomValues)
{
    DETOUR(ABuildingFoundation, PostUpdate, Anchor::Hook()
        .If(DETOUR_ORIGINAL(ABuildingFoundation, SetDynamicFoundationTransform) || DETOUR_ORIGINAL(ABuildingFoundation, execSetDynamicFoundationTransform))
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Log(TEXT("ABuildingFoundation::SelectAndStreamInLevel"))
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Start()
        .Or()
        .If(DETOUR_ORIGINAL(ABuildingFoundation, SetDynamicFoundationTransform) || DETOUR_ORIGINAL(ABuildingFoundation, execSetDynamicFoundationTransform))
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Caller()
        .Log(TEXT("ABuildingFoundation::SelectAndStreamInLevel"))
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Start()
        .Inline());

    if (!TransformIsSet())
        SetDynamicFoundationTransform(GetTransform());

    Original(PersistentState, ReservedRandomValues);

    static auto FoundationEnabledState = GetClass()->FindPropertyByName(FName(TEXT("FoundationEnabledState")));

    if (FoundationEnabledState && *FoundationEnabledState->ContainerPtrToValuePtr<EDynamicFoundationEnabledState>(this) == EDynamicFoundationEnabledState::Disabled && GetLevelToStream() != NAME_None)
        StreamOutMyBuilding();
}

void ABuildingFoundation::SetDynamicFoundationEnabled(bool bEnabled)
{
    static auto FoundationEnabledState = GetClass()->FindPropertyByName(FName(TEXT("FoundationEnabledState")));
    static auto DynamicFoundationRepData = GetClass()->FindPropertyByName(FName(TEXT("DynamicFoundationRepData")));
    static auto bFoundationEnabled = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bFoundationEnabled")));
    static auto bConditionalFoundation = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bConditionalFoundation")));
    static auto Offset = Anchor::Offset(TEXT("ABuildingFoundation::bPostUpdateCalled"))
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Log(TEXT("ABuildingFoundation::SelectAndStreamInLevel"))
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Find("or byte ptr [*+0x*], 0x*")
        .Or()
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Caller()
        .Log(TEXT("ABuildingFoundation::SelectAndStreamInLevel"))
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Find("or byte ptr [*+0x*], 0x*")
        .Disp();

    static auto Mask = (uint8_t)Anchor::Offset(TEXT("ABuildingFoundation::bPostUpdateCalled"))
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Find("or byte ptr [*+0x*], 0x*")
        .Or()
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Caller()
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Find("or byte ptr [*+0x*], 0x*")
        .Value();

    FlushNetDormancy();

    if (FoundationEnabledState)
    {
        auto& State = *FoundationEnabledState->ContainerPtrToValuePtr<EDynamicFoundationEnabledState>(this);
        State = bEnabled ? EDynamicFoundationEnabledState::Enabled : EDynamicFoundationEnabledState::Disabled;

        if (DynamicFoundationRepData)
        {
            DynamicFoundationRepData->ContainerPtrToValuePtr<FDynamicBuildingFoundationRepData>(this)->GetEnabledState() = State;
            OnRep_DynamicFoundationRepData();
        }
        else
        {
            OnRep_FoundationEnabledState();
        }
    }
    else
    {
        auto bOldEnabled = bFoundationEnabled->GetPropertyValue_InContainer(this);
        bFoundationEnabled->SetPropertyValue_InContainer(this, bEnabled);
        OnRep_FoundationEnabled(bOldEnabled);
    }

    if (!(*((uint8_t*)this + Offset) & Mask) || bConditionalFoundation->GetPropertyValue_InContainer(this) || bEnabled == (GetLevelToStream() != NAME_None))
        return;

    if (!bEnabled)
    {
        StreamOutMyBuilding();
        return;
    }

    if (auto GameState = Cast<AFortGameState>(GetWorld()->GetGameState()))
    {
        if (auto WorldManager = GameState->GetWorldManager())
            SelectAndStreamInLevel(&WorldManager->GetFoundationBuildingActorsRRV());
    }
}

void ABuildingFoundation::execSetDynamicFoundationEnabled(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(ABuildingFoundation, execSetDynamicFoundationEnabled, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.BuildingFoundation:SetDynamicFoundationEnabled"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.BuildingFoundation:SetDynamicFoundationEnabled"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.BuildingFoundation:SetDynamicFoundationEnabled")));

    uint32 bEnabled = 0;

    Stack.StepCompiledIn(&bEnabled);
    Stack.Code += !!Stack.Code;

    SetDynamicFoundationEnabled(!!bEnabled);
}

void ABuildingFoundation::SelectAndStreamInLevel(const FReservedRandomValues* ReservedRandomValues)
{
    static auto SelectAndStreamInLevel = Anchor::Function<void (*)(ABuildingFoundation*, const FReservedRandomValues*)>(TEXT("ABuildingFoundation::SelectAndStreamInLevel"))
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Target()
        .Or()
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Caller()
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Target()
        .Get();

    SelectAndStreamInLevel(this, ReservedRandomValues);
}

bool ABuildingFoundation::StreamOutMyBuilding()
{
    static auto bNeverStreamOut = GetClass()->FindPropertyByName(FName(TEXT("bNeverStreamOut")));

    if (!bNeverStreamOut || !bNeverStreamOut->GetOffset_ForInternal())
        return false;

    static auto StreamOutMyBuilding = Anchor::Function<bool (*)(ABuildingFoundation*)>(TEXT("ABuildingFoundation::StreamOutMyBuilding"))
        .FieldPattern("80 B9 %o 00 48 8B D9", bNeverStreamOut)
        .Start()
        .Get();

    return StreamOutMyBuilding(this);
}

void ABuildingFoundation::OnRep_DynamicFoundationTransform()
{
    static auto OnRep_DynamicFoundationTransform = Anchor::Function<void (*)(ABuildingFoundation*)>(TEXT("ABuildingFoundation::OnRep_DynamicFoundationTransform"))
        .Exec(TEXT("/Script/FortniteGame.BuildingFoundation:OnRep_DynamicFoundationTransform"))
        .Find("jmp 0x*")
        .Target()
        .Get();

    OnRep_DynamicFoundationTransform(this);
}

void ABuildingFoundation::OnRep_DynamicFoundationRepData()
{
    static auto OnRep_DynamicFoundationRepData = Anchor::Function<void (*)(ABuildingFoundation*)>(TEXT("ABuildingFoundation::OnRep_DynamicFoundationRepData"))
        .Exec(TEXT("/Script/FortniteGame.BuildingFoundation:OnRep_DynamicFoundationRepData"))
        .Find("jmp 0x*")
        .Target()
        .Get();

    OnRep_DynamicFoundationRepData(this);
}

void ABuildingFoundation::OnRep_FoundationEnabledState()
{
    static auto OnRep_FoundationEnabledState = Anchor::Function<void (*)(ABuildingFoundation*)>(TEXT("ABuildingFoundation::OnRep_FoundationEnabledState"))
        .Exec(TEXT("/Script/FortniteGame.BuildingFoundation:OnRep_FoundationEnabledState"))
        .Find("jmp 0x*")
        .Target()
        .Get();

    OnRep_FoundationEnabledState(this);
}

void ABuildingFoundation::OnRep_FoundationEnabled(bool bOldEnabled)
{
    static auto bTakesOldEnabled = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.BuildingFoundation:OnRep_FoundationEnabled"))
        .Has("setnz dl; call 0x*")
        .Get();

    if (bTakesOldEnabled)
    {
        static auto OnRep_FoundationEnabled = Anchor::Function<void (*)(ABuildingFoundation*, bool)>(TEXT("ABuildingFoundation::OnRep_FoundationEnabled"))
            .Exec(TEXT("/Script/FortniteGame.BuildingFoundation:OnRep_FoundationEnabled"))
            .Find("setnz dl; call 0x*")
            .Find("call 0x*")
            .Target()
            .Get();

        OnRep_FoundationEnabled(this, bOldEnabled);
        return;
    }

    static auto OnRep_FoundationEnabled = Anchor::Function<void (*)(ABuildingFoundation*)>(TEXT("ABuildingFoundation::OnRep_FoundationEnabled"))
        .Exec(TEXT("/Script/FortniteGame.BuildingFoundation:OnRep_FoundationEnabled"))
        .Find("jmp 0x*")
        .Target()
        .Get();

    OnRep_FoundationEnabled(this);
}

bool ABuildingFoundation::TransformIsSet() const
{
    static auto Offset = Anchor::Offset(TEXT("ABuildingFoundation::TransformIsSet"))
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Log(TEXT("ABuildingFoundation::SelectAndStreamInLevel"))
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Last("test byte ptr [*+0x*], 0x*")
        .Or()
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Caller()
        .Log(TEXT("ABuildingFoundation::SelectAndStreamInLevel"))
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Last("test byte ptr [*+0x*], 0x*")
        .Disp();

    static auto Mask = Anchor::Offset(TEXT("ABuildingFoundation::TransformIsSet"))
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Last("test byte ptr [*+0x*], 0x*")
        .Or()
        .String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!"))
        .Start()
        .Caller()
        .Reference("call 0x*; or byte ptr [*+0x*], 0x*")
        .Last("test byte ptr [*+0x*], 0x*")
        .Value();

    static auto FoundationEnabledState = GetClass()->FindPropertyByName(FName(TEXT("FoundationEnabledState")));

    if (!FoundationEnabledState)
        return GetDynamicFoundationType() == EDynamicFoundationType::Static || (*((uint8_t*)this + Offset) & Mask);

    switch (GetDynamicFoundationType())
    {
    case EDynamicFoundationType::Static:
    case EDynamicFoundationType::StartEnabled_Stationary:
        return true;
    case EDynamicFoundationType::StartEnabled_Dynamic:
        return (*((uint8_t*)this + Offset) & Mask) != 0;
    case EDynamicFoundationType::StartDisabled:
        return *FoundationEnabledState->ContainerPtrToValuePtr<EDynamicFoundationEnabledState>(this) == EDynamicFoundationEnabledState::Enabled;
    }

    return false;
}

EDynamicFoundationType ABuildingFoundation::GetDynamicFoundationType() const
{
    static auto DynamicFoundationType = GetClass()->FindPropertyByName(FName(TEXT("DynamicFoundationType")));

    return *DynamicFoundationType->ContainerPtrToValuePtr<EDynamicFoundationType>(this);
}

FName ABuildingFoundation::GetLevelToStream() const
{
    static auto LevelToStream = GetClass()->FindPropertyByName(FName(TEXT("LevelToStream")));

    return LevelToStream->GetPropertyValue_InContainer<FName>(this);
}

FTransform& ABuildingFoundation::GetDynamicFoundationTransform()
{
    static auto DynamicFoundationTransform = GetClass()->FindPropertyByName(FName(TEXT("DynamicFoundationTransform")));

    return *DynamicFoundationTransform->ContainerPtrToValuePtr<FTransform>(this);
}
