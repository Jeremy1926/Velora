#include "pch.h"
#include "Building/BuildingFoundation.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
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

static auto SetDynamicFoundationTransform = []
{
    auto Call = Scanner::Exec(TEXT("/Script/FortniteGame.BuildingFoundation:SetDynamicFoundationTransform")).Find("mov [*+0x20], *; call 0x*").Find("call 0x*");

    return Scanner::At(Call.Target()).Is("ret*") ? DetourCall(TEXT("ABuildingFoundation::SetDynamicFoundationTransform"), Call.Get(), &ABuildingFoundation::SetDynamicFoundationTransform) : nullptr;
}();

static auto PostUpdate = ::SetDynamicFoundationTransform ? Detour(TEXT("ABuildingFoundation::PostUpdate"), PostUpdateCall().Start().Get(), &ABuildingFoundation::PostUpdate) : nullptr;

void ABuildingFoundation::PostUpdate(EFortBuildingPersistentState PersistentState, const FReservedRandomValues* ReservedRandomValues)
{
    if (!TransformIsSet())
        SetDynamicFoundationTransform(GetTransform());

    (this->*::PostUpdate)(PersistentState, ReservedRandomValues);
}

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

void ABuildingFoundation::OnRep_DynamicFoundationTransform()
{
    static auto OnRep_DynamicFoundationTransform = Scanner::Function<void (*)(ABuildingFoundation*)>(Scanner::Exec(TEXT("/Script/FortniteGame.BuildingFoundation:OnRep_DynamicFoundationTransform")).Find("jmp 0x*").Target(TEXT("ABuildingFoundation::OnRep_DynamicFoundationTransform")));

    OnRep_DynamicFoundationTransform(this);
}

void ABuildingFoundation::OnRep_DynamicFoundationRepData()
{
    static auto OnRep_DynamicFoundationRepData = Scanner::Function<void (*)(ABuildingFoundation*)>(Scanner::Exec(TEXT("/Script/FortniteGame.BuildingFoundation:OnRep_DynamicFoundationRepData")).Find("jmp 0x*").Target(TEXT("ABuildingFoundation::OnRep_DynamicFoundationRepData")));

    OnRep_DynamicFoundationRepData(this);
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

FTransform& ABuildingFoundation::GetDynamicFoundationTransform()
{
    static auto DynamicFoundationTransform = GetClass()->FindPropertyByName(FName(TEXT("DynamicFoundationTransform")));

    return *DynamicFoundationTransform->ContainerPtrToValuePtr<FTransform>(this);
}
