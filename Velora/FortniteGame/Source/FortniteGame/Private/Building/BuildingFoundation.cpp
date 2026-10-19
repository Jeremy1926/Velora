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

    FlushNetDormancy();

    GetDynamicFoundationTransform() = NewTransform;
    *((uint8_t*)this + Offset) |= Mask;

    OnRep_DynamicFoundationTransform();
}

void ABuildingFoundation::OnRep_DynamicFoundationTransform()
{
    static auto OnRep_DynamicFoundationTransform = Scanner::Function<void (*)(ABuildingFoundation*)>(Scanner::Exec(TEXT("/Script/FortniteGame.BuildingFoundation:OnRep_DynamicFoundationTransform")).Find("jmp 0x*").Target(TEXT("ABuildingFoundation::OnRep_DynamicFoundationTransform")));

    OnRep_DynamicFoundationTransform(this);
}

bool ABuildingFoundation::TransformIsSet() const
{
    static auto Offset = TransformIsSetFlag().Disp(TEXT("ABuildingFoundation::TransformIsSet"));
    static auto Mask = TransformIsSetFlag().Imm();

    return GetDynamicFoundationType() == EDynamicFoundationType::Static || (*((uint8_t*)this + Offset) & Mask);
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
