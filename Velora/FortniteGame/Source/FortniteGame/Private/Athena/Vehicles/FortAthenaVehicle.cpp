#include "pch.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Athena/Vehicles/FortVehicleConfigs.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/EngineTypes.h"
#include "GameplayEffectTypes.h"
#include "Math/UnrealMathUtility.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/BodySetup.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectArray.h"
#include "UObject/Stack.h"
#include "Detour.h"
#include "Scanner.h"

UClass* AFortAthenaVehicle::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicle"));

    return Class;
}

void AFortAthenaVehicle::InitializeServerImplementations()
{
    static auto ServerUpdatePhysicsParams = (UFunction*)StaticFindObject(nullptr, nullptr, GetServerFunctionPath(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPhysicsPawn:ServerMove")) ? TEXT("ServerMove") : TEXT("ServerUpdatePhysicsParams")));

    if (!ServerUpdatePhysicsParams)
        return;

    static auto ServerUpdatePhysicsParams_Implementation = THook(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"), &AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation)
        .Exec(ServerUpdatePhysicsParams)
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"))
        .Or()
        .Exec(ServerUpdatePhysicsParams)
        .Find("call [rax+*]")
        .LogSlot(TEXT("AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation"))
        .InChildVTables((UClass*)ServerUpdatePhysicsParams->GetOuter());

    static auto ServerUpdateInputParams_Implementation = THook(TEXT("AFortAthenaVehicle::ServerUpdateInputParams_Implementation"), &AFortAthenaVehicle::ServerUpdateInputParams_Implementation)
        .Exec(GetServerFunctionPath(TEXT("ServerUpdateInputParams")))
        .Optional()
        .Find("call [rax+*]", 2)
        .LogSlot(TEXT("AFortAthenaVehicle::ServerUpdateInputParams_Implementation"))
        .InChildVTables((UClass*)ServerUpdatePhysicsParams->GetOuter());
}

const TCHAR* AFortAthenaVehicle::GetServerFunctionPath(const TCHAR* FunctionName)
{
    static auto Owner = StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPhysicsPawn:ServerUpdatePhysicsParams")) || StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPhysicsPawn:ServerMove")) ? TEXT("FortPhysicsPawn") : StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaSMVehicle:ServerUpdatePhysicsParams")) ? TEXT("FortAthenaSMVehicle") : TEXT("FortAthenaVehicle");
    static TCHAR Path[256];

    swprintf_s(Path, TEXT("/Script/FortniteGame.%s:%s"), Owner, FunctionName);

    return Path;
}

void AFortAthenaVehicle::ServerUpdatePhysicsParams_Implementation(const FReplicatedAthenaVehiclePhysicsState& InState)
{
    static auto bEncodedRotation = []
    {
        auto PhysicsState = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ReplicatedPhysicsPawnState"));

        if (!PhysicsState)
            PhysicsState = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ReplicatedAthenaVehiclePhysicsState"));

        return PhysicsState->FindPropertyByName(FName(TEXT("SyncKey"))) != nullptr;
    }();

    static auto EncodedRotationX = Anchor::Address()
        .Pattern("F3 0F 58 2D ? ? ? ? F3 0F 58 0D")
        .Target()
        .Or()
        .Pattern("F3 0F 58 2D ? ? ? ? E9")
        .Target()
        .Get();

    auto PrimitiveComponent = Cast<UPrimitiveComponent>(GetRootComponent());

    if (!PrimitiveComponent)
        return;

    auto Rotation = !bEncodedRotation ? InState.Rotation : EncodedRotationX && *(const float*)EncodedRotationX == 0.3f ? FQuat(InState.Rotation.X - 0.3f, InState.Rotation.Y / -0.75f, InState.Rotation.Z + 0.15f, InState.Rotation.W / 1.1f) : FQuat(InState.Rotation.X - 2.5f, InState.Rotation.Y / 0.3f, InState.Rotation.Z + 2.0f, InState.Rotation.W / -1.2f);

    PrimitiveComponent->K2_SetWorldLocationAndRotation(InState.Translation, Rotation.Rotator(), false, true);
    PrimitiveComponent->SetPhysicsLinearVelocity(InState.LinearVelocity, false, NAME_None);
    PrimitiveComponent->SetPhysicsAngularVelocityInRadians(InState.AngularVelocity, false, NAME_None);
}

void AFortAthenaVehicle::ServerUpdateInputParams_Implementation(const FReplicatedAthenaVehicleInputState& InState)
{
    static auto ReplicatedInputParams = GetClass()->FindPropertyByName(FName(TEXT("ReplicatedInputParams")));
    static auto LocalInputParams = GetClass()->FindPropertyByName(FName(TEXT("LocalInputParams")));

    memcpy(ReplicatedInputParams->ContainerPtrToValuePtr<void>(this), &InState, ReplicatedInputParams->GetSize());
    memcpy(LocalInputParams->ContainerPtrToValuePtr<void>(this), &InState, LocalInputParams->GetSize());

    WakeUp();
}

void AFortAthenaVehicle::WakeUp()
{
    static auto WakeUp = Anchor::VirtualFunction<void (*)(AFortAthenaVehicle*)>(TEXT("AFortAthenaVehicle::WakeUp"))
        .Exec(GetServerFunctionPath(TEXT("OnRep_InputParams")))
        .Find("jmp *")
        .Is("jmp [rax+*]")
        .Or()
        .Exec(GetServerFunctionPath(TEXT("OnRep_InputParams")))
        .Find("jmp *")
        .Target()
        .Find("jmp [rax+*]")
        .Get();

    WakeUp(this);
}

AFortPlayerPawn* AFortAthenaVehicle::GetPawnAtSeat(int32 SeatIdx) const
{
    static auto VirtualGetPawnAtSeat = Anchor::VirtualFunction<AFortPlayerPawn* (*)(const AFortAthenaVehicle*, int32)>(TEXT("AFortAthenaVehicle::GetPawnAtSeat"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat"))
        .Find("setnz *")
        .Find("mov [*+0x20], *")
        .Find("call *")
        .Is("call [*]")
        .Optional()
        .Get();

    if (VirtualGetPawnAtSeat)
        return VirtualGetPawnAtSeat(this, SeatIdx);

    static auto GetPawnAtSeat = Anchor::Function<AFortPlayerPawn* (*)(const AFortAthenaVehicle*, int32)>(TEXT("AFortAthenaVehicle::GetPawnAtSeat"))
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat"))
        .Find("setnz *")
        .Find("mov [*+0x20], *")
        .Find("call *")
        .Is("call 0x*")
        .Target()
        .Get();

    return GetPawnAtSeat(this, SeatIdx);
}

float AFortAthenaVehicle::ModifyIncomingDamage(float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser)
{
    DETOUR(AFortAthenaVehicle, ModifyIncomingDamage, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts")) != nullptr)
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:ModifyIncomingDamage"))
        .Find("call [*+0x*]")
        .LogSlot(TEXT("AFortAthenaVehicle::ModifyIncomingDamage"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortAthenaVehicle"))
        .Optional()
        .Inline());

    const float ModifiedDamage = Original(Damage, InTags, EffectContext, EventInstigator, DamageCauser);

    if (ModifiedDamage > 0.0f)
        AbsorbDamageIntoDamagablePart(Damage, EffectContext);

    return ModifiedDamage;
}

void AFortAthenaVehicle::AbsorbDamageIntoDamagablePart(float Damage, const FGameplayEffectContextHandle& EffectContext)
{
    auto HitResult = EffectContext.GetHitResult();
    auto BulletCollisionComponent = GetVehicleBulletCollisionComponent();

    if (!HasAuthority() || !HitResult || !BulletCollisionComponent || HitResult->GetComponent().Get() != BulletCollisionComponent)
        return;

    auto& DamageableParts = GetDamageableParts();

    for (int32 Index = 0; Index < DamageableParts.Num(); Index++)
    {
        if (DamageableParts[Index].ShapeIndex != HitResult->GetElementIndex() || DamageableParts[Index].Health <= 0.0f)
            continue;

        const TArray<FVehicleDamageablePart> PrevDamageableParts = DamageableParts;

        DamageableParts[Index].Health = FMath::Max(DamageableParts[Index].Health - Damage, 0.0f);

        OnRep_DamageableParts(PrevDamageableParts);
        ForceNetUpdate();

        return;
    }
}

void AFortAthenaVehicle::DestroyAllDamageableParts()
{
    auto& DamageableParts = GetDamageableParts();
    const TArray<FVehicleDamageablePart> PrevDamageableParts = DamageableParts;

    for (auto& Part : DamageableParts)
    {
        if (Part.Health > 0.0f)
            Part.Health = 0.0f;
    }

    OnRep_DamageableParts(PrevDamageableParts);
    ForceNetUpdate();
}

void AFortAthenaVehicle::execDestroyAllDamageableParts(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortAthenaVehicle, execDestroyAllDamageableParts, Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:DestroyAllDamageableParts"))
        .Find("mov [*+0x20], *; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortAthenaVehicle:DestroyAllDamageableParts"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortAthenaVehicle:DestroyAllDamageableParts")));

    Stack.Code += !!Stack.Code;

    DestroyAllDamageableParts();
}

void AFortAthenaVehicle::RepairAllDamageableParts()
{
    DETOUR(AFortAthenaVehicle, RepairAllDamageableParts, Anchor::Hook()
        .If(Anchor::Address()
            .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:RepairAllDamageableParts"))
            .Find("jmp [*+0x*]")
            .Entry(TEXT("/Script/FortniteGame.Default__FortAthenaVehicle"))
            .Is("ret*")
            .Get() != nullptr)
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:RepairAllDamageableParts"))
        .Find("jmp [*+0x*]")
        .LogSlot(TEXT("AFortAthenaVehicle::RepairAllDamageableParts"))
        .InChildVTables(AFortAthenaVehicle::StaticClass()), Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:RepairAllDamageableParts"))
        .Find("jmp [*+0x*]")
        .Entry(TEXT("/Script/ValetRuntime.Default__FortDagwoodVehicle"))
        .Find("call 0x*")
        .IsTarget("ret*")
        .Optional()
        .AtCall());

    static auto FortVehicleInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleInterface"));

    auto VehicleInterface = HasAuthority() ? (IFortVehicleInterface*)GetInterfaceAddress(FortVehicleInterface) : nullptr;
    auto VehicleConfigs = VehicleInterface ? VehicleInterface->GetVehicleConfigs() : nullptr;

    if (!VehicleConfigs)
        return;

    auto& DamageableParts = GetDamageableParts();
    const auto& PartConfigs = VehicleConfigs->GetDamageableParts();
    const TArray<FVehicleDamageablePart> PrevDamageableParts = DamageableParts;

    for (auto& Part : DamageableParts)
    {
        if (PartConfigs.IsValidIndex(Part.ConfigIndex))
            Part.Health = PartConfigs[Part.ConfigIndex].MaxHealth;
    }

    OnRep_DamageableParts(PrevDamageableParts);
    ForceNetUpdate();
}

void AFortAthenaVehicle::SetupToggleableShapeCollision()
{
    DETOUR(AFortAthenaVehicle, SetupToggleableShapeCollision, Anchor::Hook()
        .Body(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts"))
        .VirtualIs(TEXT("/Script/FortniteGame.Default__FortAthenaVehicle"), "lea rdx, [rcx+0x*]; jmp %t")
        .LogSlot(TEXT("AFortAthenaVehicle::SetupToggleableShapeCollision"))
        .Optional()
        .Inline());

    static auto FortVehicleInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleInterface"));

    auto& DamageableParts = GetDamageableParts();
    auto VehicleInterface = (IFortVehicleInterface*)GetInterfaceAddress(FortVehicleInterface);
    auto VehicleConfigs = HasAuthority() && !DamageableParts.Num() && VehicleInterface ? VehicleInterface->GetVehicleConfigs() : nullptr;
    auto BulletCollisionComponent = VehicleConfigs ? GetVehicleBulletCollisionComponent() : nullptr;
    auto BodyInstance = BulletCollisionComponent ? BulletCollisionComponent->GetBodyInstance(NAME_None, true) : nullptr;
    auto BodySetup = BodyInstance ? BodyInstance->GetBodySetup() : nullptr;

    if (BodySetup)
    {
        auto SkeletalMeshComponent = VehicleInterface->GetSkeletalMeshComponent();
        const auto& PartConfigs = VehicleConfigs->GetDamageableParts();

        for (int32 ConfigIndex = 0; ConfigIndex < PartConfigs.Num(); ConfigIndex++)
        {
            const int32 ShapeIndex = BodySetup->GetAggGeom().GetElementIndexByName(PartConfigs[ConfigIndex].ShapeName);

            if (ShapeIndex == INDEX_NONE)
                continue;

            FVehicleDamageablePart Part;
            Part.ConfigIndex = ConfigIndex;
            Part.BoneIndex = SkeletalMeshComponent ? SkeletalMeshComponent->GetBoneIndex(PartConfigs[ConfigIndex].BoneName) : INDEX_NONE;
            Part.ShapeIndex = ShapeIndex;
            Part.Health = PartConfigs[ConfigIndex].MaxHealth;

            DamageableParts.Add(Part);
        }
    }

    Original();
}

void AFortAthenaVehicle::OnRep_DamageableParts(const TArray<FVehicleDamageablePart>& PrevDamageableParts)
{
    static auto OnRep_DamageableParts = Anchor::Function<void (*)(AFortAthenaVehicle*, const TArray<FVehicleDamageablePart>&)>(TEXT("AFortAthenaVehicle::OnRep_DamageableParts"))
        .Body(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts"))
        .Get();

    OnRep_DamageableParts(this, PrevDamageableParts);
}

TArray<FVehicleDamageablePart>& AFortAthenaVehicle::GetDamageableParts()
{
    static auto DamageableParts = StaticClass()->FindPropertyByName(FName(TEXT("DamageableParts")));

    return *DamageableParts->ContainerPtrToValuePtr<TArray<FVehicleDamageablePart>>(this);
}

UPrimitiveComponent* AFortAthenaVehicle::GetVehicleBulletCollisionComponent() const
{
    static auto GetVehicleBulletCollisionComponent = Anchor::Function<UPrimitiveComponent* (*)(const AFortAthenaVehicle*)>(TEXT("AFortAthenaVehicle::GetVehicleBulletCollisionComponent"))
        .Body(TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts"))
        .Find("call 0x*")
        .Target()
        .Get();

    return GetVehicleBulletCollisionComponent(this);
}
