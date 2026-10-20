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

static Scanner::FCode PostUpdateCall()
{
    static auto SelectAndStreamInLevel = Scanner::String(TEXT("ABuildingFoundation::SelectAndSetupMyBuildingLevel - Cannot get WorldManager!!")).Start().Get(TEXT("ABuildingFoundation::SelectAndStreamInLevel"));
    static auto PostUpdateCall = Scanner::Reference(SelectAndStreamInLevel, "call 0x*; or byte ptr [*+0x*], 0x*");

    return PostUpdateCall;
}

static Scanner::FCode TransformIsSetFlag()
{
    static auto TransformIsSetFlag = PostUpdateCall().Last("test byte ptr [*+0x*], 0x*");

    return TransformIsSetFlag;
}

static Scanner::FCode PostUpdateCalledFlag()
{
    static auto PostUpdateCalledFlag = PostUpdateCall().Find("or byte ptr [*+0x*], 0x*");

    return PostUpdateCalledFlag;
}

DETOUR(ABuildingFoundation, SetDynamicFoundationTransform, Anchor::Hook()
    .Exec(TEXT("/Script/FortniteGame.BuildingFoundation:SetDynamicFoundationTransform"))
    .Find("mov [*+0x20], *; call 0x*")
    .Find("call 0x*")
    .IsTarget("ret*")
    .Optional()
    .AtCall())
void ABuildingFoundation::SetDynamicFoundationTransform(const FTransform& NewTransform)
{
    static auto Offset = TransformIsSetFlag().Disp();
    static auto Mask = (uint8_t)TransformIsSetFlag().Imm();
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
    RepData.Translation = NewTransform.GetTranslation();
    RepData.Rotation = NewTransform.GetRotation();

    OnRep_DynamicFoundationRepData();
}

DETOUR(ABuildingFoundation, PostUpdate, Anchor::Hook()
    .If(DETOUR_ORIGINAL(ABuildingFoundation, SetDynamicFoundationTransform))
    .At(PostUpdateCall())
    .Start()
    .Inline())
void ABuildingFoundation::PostUpdate(EFortBuildingPersistentState PersistentState, const FReservedRandomValues* ReservedRandomValues)
{
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
    static auto Offset = PostUpdateCalledFlag().Disp(TEXT("ABuildingFoundation::bPostUpdateCalled"));
    static auto Mask = (uint8_t)PostUpdateCalledFlag().Imm();

    FlushNetDormancy();

    if (FoundationEnabledState)
    {
        auto& State = *FoundationEnabledState->ContainerPtrToValuePtr<EDynamicFoundationEnabledState>(this);
        State = bEnabled ? EDynamicFoundationEnabledState::Enabled : EDynamicFoundationEnabledState::Disabled;

        if (DynamicFoundationRepData)
        {
            DynamicFoundationRepData->ContainerPtrToValuePtr<FDynamicBuildingFoundationRepData>(this)->EnabledState = State;
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

DETOUR(ABuildingFoundation, execSetDynamicFoundationEnabled, Anchor::Hook()
    .Native(TEXT("/Script/FortniteGame.BuildingFoundation:SetDynamicFoundationEnabled"))
    .Is("ret*")
    .Optional()
    .InExec(TEXT("/Script/FortniteGame.BuildingFoundation:SetDynamicFoundationEnabled")))
void ABuildingFoundation::execSetDynamicFoundationEnabled(FFrame& Stack, void* const Z_Param__Result)
{
    uint32 bEnabled = 0;

    Stack.StepCompiledIn(&bEnabled);
    Stack.Code += !!Stack.Code;

    SetDynamicFoundationEnabled(!!bEnabled);
}

void ABuildingFoundation::SelectAndStreamInLevel(const FReservedRandomValues* ReservedRandomValues)
{
    static auto SelectAndStreamInLevel = Scanner::Function<void (*)(ABuildingFoundation*, const FReservedRandomValues*)>(PostUpdateCall().Target());

    SelectAndStreamInLevel(this, ReservedRandomValues);
}

bool ABuildingFoundation::StreamOutMyBuilding()
{
    static auto bNeverStreamOut = GetClass()->FindPropertyByName(FName(TEXT("bNeverStreamOut")));
    static auto Offset = bNeverStreamOut ? bNeverStreamOut->GetOffset_ForInternal() : 0;
    static auto StreamOutMyBuilding = Scanner::Function<bool (*)(ABuildingFoundation*)>(Offset ? Scanner::Pattern(std::format("80 B9 {:02X} {:02X} {:02X} {:02X} 00 48 8B D9", Offset & 0xFF, (Offset >> 8) & 0xFF, (Offset >> 16) & 0xFF, (Offset >> 24) & 0xFF).c_str()).Start().Get(TEXT("ABuildingFoundation::StreamOutMyBuilding")) : nullptr);

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
    static auto Exec = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingFoundation:OnRep_FoundationEnabled"));
    static auto bTakesOldEnabled = Exec.Has("setnz dl; call 0x*");

    if (bTakesOldEnabled)
    {
        static auto OnRep_FoundationEnabled = Anchor::Function<void (*)(ABuildingFoundation*, bool)>(TEXT("ABuildingFoundation::OnRep_FoundationEnabled"))
            .At(Exec)
            .Find("setnz dl; call 0x*")
            .Find("call 0x*")
            .Target()
            .Get();

        OnRep_FoundationEnabled(this, bOldEnabled);
        return;
    }

    static auto OnRep_FoundationEnabled = Anchor::Function<void (*)(ABuildingFoundation*)>(TEXT("ABuildingFoundation::OnRep_FoundationEnabled"))
        .At(Exec)
        .Find("jmp 0x*")
        .Target()
        .Get();

    OnRep_FoundationEnabled(this);
}

bool ABuildingFoundation::TransformIsSet() const
{
    static auto Offset = TransformIsSetFlag().Disp(TEXT("ABuildingFoundation::TransformIsSet"));
    static auto Mask = TransformIsSetFlag().Imm();
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

    return *LevelToStream->ContainerPtrToValuePtr<FName>(this);
}

FTransform& ABuildingFoundation::GetDynamicFoundationTransform()
{
    static auto DynamicFoundationTransform = GetClass()->FindPropertyByName(FName(TEXT("DynamicFoundationTransform")));

    return *DynamicFoundationTransform->ContainerPtrToValuePtr<FTransform>(this);
}
