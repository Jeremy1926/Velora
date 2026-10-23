#include "pch.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Athena/Vehicles/FortVehicleConfigs.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "Pawns/FortPlayerPawn.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Controller.h"
#include "GameplayTagContainer.h"
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

TMulticastDelegate<void, AActor*>& AFortAthenaVehicle::GetNotifyNetCullDistanceChange()
{
    static auto NotifyNetCullDistanceChange = Anchor::Data<TMulticastDelegate<void, AActor*>*>(TEXT("AFortAthenaVehicle::NotifyNetCullDistanceChange"))
        .FieldPattern("F3 0F 59 C0 48 8D 0D ? ? ? ? 48 8B D3 F3 0F 11 83 %o", AActor::StaticClass() ? AActor::StaticClass()->FindPropertyByName(FName(TEXT("NetCullDistanceSquared"))) : nullptr)
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return *NotifyNetCullDistanceChange;
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
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:GetPawnAtSeat"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
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

bool AFortAthenaVehicle::ForceEnterSeat(AFortPlayerPawn* Pawn, EVehicleSeats Seat, bool bIgnoreDBNOCheck)
{
    static auto ForceEnterSeatBody = Anchor::Address()
        .Pattern("48 89 5C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 83 EC 40 48 8B 01 48 8B FA 4C 8B F1 FF 90 ? ? ? ?")
        .Has("lea rdx, [rax+0x*]; call [rbx+0x*]; test al, al; jz 0x*; mov rax, [rsi]; xor r8d, r8d; mov rdx, rdi; mov rcx, rsi; call [rax+0x*]")
        .Or()
        .Pattern("48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 40 48 8B 01 48 8B FA 41 0F B6 E8 4C 8B F1 FF 90 ? ? ? ?")
        .Has("lea rdx, [rax+0x*]; call rbx; test al, al; jz 0x*; mov rax, [rsi]; mov r8d, ebp; mov rdx, rdi; mov rcx, rsi; call [rax+0x*]")
        .Optional()
        .Get();

    if (!ForceEnterSeatBody)
        return false;

    static auto CanContainPlayers = Anchor::VirtualFunction<bool (*)(const IFortVehicleInterface*)>(TEXT("IFortVehicleInterface::CanContainPlayers"))
        .At(ForceEnterSeatBody)
        .Find("lea rsi, [r14+0x*]; mov rax, [rsi]; mov rcx, rsi; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    static auto GameplayTags = Anchor::Function<const uint8* (*)()>(TEXT("UFortGlobals::GameplayTags"))
        .At(ForceEnterSeatBody)
        .Find("call 0x*; mov rcx, rdi; lea rdx, [rax+0x*]")
        .Target()
        .Get();

    static auto AthenaEnterVehicle = Anchor::Offset(TEXT("FFortGlobalGameplayTags::AthenaEnterVehicle"))
        .At(ForceEnterSeatBody)
        .Find("call 0x*; mov rcx, rdi; lea rdx, [rax+0x*]")
        .Find("lea rdx, [rax+0x*]")
        .Disp();

    static auto CanPerformNativeAction = Anchor::VirtualFunction<bool (*)(const AFortPlayerPawn*, const FGameplayTag*)>(TEXT("AFortPawn::CanPerformNativeAction"))
        .At(ForceEnterSeatBody)
        .Find("lea rdx, [rax+0x*]; call [rbx+0x*]")
        .Find("call [rbx+0x*]")
        .Or()
        .At(ForceEnterSeatBody)
        .Find("mov rax, [rdi]; mov rbx, [rax+0x*]")
        .Find("mov rbx, [rax+0x*]")
        .Get();

    static auto CanPutPlayerInSlot = Anchor::VirtualFunction<bool (*)(const IFortVehicleInterface*, AFortPlayerPawn*, int32)>(TEXT("IFortVehicleInterface::CanPutPlayerInSlot"))
        .At(ForceEnterSeatBody)
        .Find("mov rdx, rdi; mov rcx, rsi; call [rax+0x*]; test al, al")
        .Find("call [rax+0x*]")
        .Get();

    static auto SeatPlayerInternal = Anchor::VirtualFunction<void (*)(IFortVehicleInterface*, AFortPlayerPawn*, FName&, int32)>(TEXT("IFortVehicleInterface::SeatPlayerInternal"))
        .At(ForceEnterSeatBody)
        .Find("lea r8, [rsp+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    static auto bInheritScale = Anchor::Offset(TEXT("UFortVehicleConfigs::bInheritScale"))
        .At(ForceEnterSeatBody)
        .Find("cmp byte ptr [rax+0x*], 0x0; cmovz r8, rcx")
        .Disp();

    static auto SnapToTargetNotIncludingScale = Anchor::Data<const FAttachmentTransformRules*>(TEXT("FAttachmentTransformRules::SnapToTargetNotIncludingScale"))
        .At(ForceEnterSeatBody)
        .Find("cmp byte ptr [rax+0x*], 0x0; cmovz r8, rcx")
        .Last("lea rcx, [0x*]")
        .Target()
        .Get();

    static auto SnapToTargetIncludingScale = Anchor::Data<const FAttachmentTransformRules*>(TEXT("FAttachmentTransformRules::SnapToTargetIncludingScale"))
        .At(ForceEnterSeatBody)
        .Find("cmp byte ptr [rax+0x*], 0x0; cmovz r8, rcx")
        .Last("lea r8, [0x*]")
        .Target()
        .Get();

    static auto AttachToActor = Anchor::Function<bool (*)(AActor*, AActor*, const FAttachmentTransformRules*, FName)>(TEXT("AActor::AttachToActor"))
        .At(ForceEnterSeatBody)
        .Find("cmovz r8, rcx; mov rcx, rdi; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto CharacterMovement = Anchor::Offset(TEXT("ACharacter::CharacterMovement"))
        .At(ForceEnterSeatBody)
        .Find("mov rbx, [rdi+0x*]; mov rcx, rbx; mov rax, [rbx]; call [rax+0x*]")
        .Disp();

    static auto StopMovementImmediately = Anchor::VirtualFunction<void (*)(UObject*)>(TEXT("UMovementComponent::StopMovementImmediately"))
        .At(ForceEnterSeatBody)
        .Find("mov rbx, [rdi+0x*]; mov rcx, rbx; mov rax, [rbx]; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    static auto ServerGetDriverPawn = Anchor::VirtualFunction<AFortPlayerPawn* (*)(const IFortVehicleInterface*)>(TEXT("IFortVehicleInterface::ServerGetDriverPawn"))
        .At(ForceEnterSeatBody)
        .Find("mov rax, [rsi]; mov rcx, rsi; call [rax+0x*]; mov r9, [rbx]")
        .Find("call [rax+0x*]")
        .Or()
        .At(ForceEnterSeatBody)
        .Find("mov rax, [rsi]; mov rcx, rsi; call [rax+0x*]; mov rcx, rbx; cmp rax, rdi")
        .Find("call [rax+0x*]")
        .Get();

    static auto SetMovementMode = Anchor::VirtualFunction<void (*)(UObject*, uint8, uint8)>(TEXT("UCharacterMovementComponent::SetMovementMode"))
        .At(ForceEnterSeatBody)
        .Find("mov edx, 0x6; mov rcx, rbx")
        .Find("call [r9+0x*]")
        .Or()
        .At(ForceEnterSeatBody)
        .Find("cmp rax, rdi; jnz 0x*; call 0x*")
        .Call()
        .Is("mov rax, [rcx]; mov r8b, 0x1; mov edx, 0x6; jmp [rax+0x*]")
        .Find("jmp [rax+0x*]")
        .Get();

    static auto SetBase = Anchor::VirtualFunction<void (*)(AFortPlayerPawn*, USceneComponent*, FName, bool)>(TEXT("ACharacter::SetBase"))
        .At(ForceEnterSeatBody)
        .Find("mov rdx, [r14+0x*]; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Get();

    static auto TryActivateAbilitiesByTag = Anchor::Function<bool (*)(UAbilitySystemComponent*, const FGameplayTagContainer&, bool)>(TEXT("UAbilitySystemComponent::TryActivateAbilitiesByTag"))
        .At(ForceEnterSeatBody)
        .Find("mov r8b, 0x1; lea rdx, [rsp+0x*]; mov rcx, rbx; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    static auto FortVehicleInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleInterface"));
    static auto FortPlayerControllerAthena = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerControllerAthena"));
    static auto bDelayedTeleporting = FortPlayerControllerAthena ? (UBoolProperty*)FortPlayerControllerAthena->FindPropertyByName(FName(TEXT("bDelayedTeleporting"))) : nullptr;
    static auto bIsUsingJetpack = (UBoolProperty*)AFortPlayerPawn::StaticClass()->FindPropertyByName(FName(TEXT("bIsUsingJetpack")));

    auto VehicleInterface = (IFortVehicleInterface*)GetInterfaceAddress(FortVehicleInterface);

    if (!VehicleInterface || !GameplayTags || !AttachToActor || !TryActivateAbilitiesByTag || !SnapToTargetIncludingScale || !SnapToTargetNotIncludingScale)
        return false;

    WakeUp();

    if (!Pawn || !CanContainPlayers(VehicleInterface))
        return false;

    auto Controller = Pawn->GetController();

    if (Controller && bDelayedTeleporting && Controller->IsA(FortPlayerControllerAthena) && bDelayedTeleporting->GetPropertyValue_InContainer(Controller))
        return false;

    if (!bIgnoreDBNOCheck && Pawn->GetIsDBNO())
        return false;

    if (Pawn->GetVehicleStateLocal().GetVehicle() || (bIsUsingJetpack && bIsUsingJetpack->GetPropertyValue_InContainer(Pawn)))
        return false;

    auto EnterVehicleTag = (const FGameplayTag*)(GameplayTags() + AthenaEnterVehicle);

    if (!CanPerformNativeAction(Pawn, EnterVehicleTag) || !CanPutPlayerInSlot(VehicleInterface, Pawn, (int32)Seat))
        return false;

    FName SeatName(NAME_None);
    SeatPlayerInternal(VehicleInterface, Pawn, SeatName, (int32)Seat);

    auto VehicleConfigs = (const uint8*)VehicleInterface->GetVehicleConfigs();
    AttachToActor(Pawn, this, VehicleConfigs && VehicleConfigs[bInheritScale] ? SnapToTargetIncludingScale : SnapToTargetNotIncludingScale, SeatName);

    auto MovementComponent = *(UObject**)((uint8*)Pawn + CharacterMovement);
    StopMovementImmediately(MovementComponent);
    SetMovementMode(MovementComponent, 6, ServerGetDriverPawn(VehicleInterface) == Pawn ? 1 : 2);
    SetBase(Pawn, GetRootComponent(), SeatName, true);

    if (auto AbilitySystemComponent = Pawn->GetAbilitySystemComponent())
    {
        FGameplayTagContainer TagContainer;
        TagContainer.AddTag(*EnterVehicleTag);

        TryActivateAbilitiesByTag(AbilitySystemComponent, TagContainer, true);
    }

    return true;
}

float AFortAthenaVehicle::ModifyIncomingDamage(float Damage, const FGameplayTagContainer& InTags, const FGameplayEffectContextHandle& EffectContext, AController* EventInstigator, AActor* DamageCauser)
{
    DETOUR(AFortAthenaVehicle, ModifyIncomingDamage, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts")) != nullptr)
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:ModifyIncomingDamage"))
        .Find("call [*+0x*]")
        .LogSlot(TEXT("AFortAthenaVehicle::ModifyIncomingDamage"))
        .Entry(TEXT("/Script/FortniteGame.Default__FortAthenaVehicle"))
        .Or()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicle:OnRep_DamageableParts")) != nullptr)
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:ModifyIncomingDamage"))
        .Find("setnz *")
        .Find("mov rdx, [rax+0x*]")
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

void AFortAthenaVehicle::RepairAllDamageableParts(float NewHealth)
{
    DETOUR(AFortAthenaVehicle, RepairAllDamageableParts, Anchor::Hook()
        .If(Anchor::Address()
            .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:RepairAllDamageableParts"))
            .Find("jmp [*+0x*]")
            .Entry(TEXT("/Script/FortniteGame.Default__FortAthenaVehicle"))
            .Is("ret*")
            .Or()
            .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:RepairAllDamageableParts"))
            .Find("setnz *; add [*+0x20], *; mov rax, [*]; call [rax+0x*]")
            .Find("call [rax+0x*]")
            .Entry(TEXT("/Script/FortniteGame.Default__FortAthenaVehicle"))
            .Is("ret*")
            .Get() != nullptr)
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:RepairAllDamageableParts"))
        .Find("jmp [*+0x*]")
        .LogSlot(TEXT("AFortAthenaVehicle::RepairAllDamageableParts"))
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:RepairAllDamageableParts"))
        .Find("setnz *; add [*+0x20], *; mov rax, [*]; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .LogSlot(TEXT("AFortAthenaVehicle::RepairAllDamageableParts"))
        .InChildVTables(AFortAthenaVehicle::StaticClass()), Anchor::Hook()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:RepairAllDamageableParts"))
        .Find("jmp [*+0x*]")
        .Entry(TEXT("/Script/ValetRuntime.Default__FortDagwoodVehicle"))
        .Find("call 0x*")
        .IsTarget("ret*")
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortAthenaVehicle:RepairAllDamageableParts"))
        .Find("setnz *; add [*+0x20], *; mov rax, [*]; call [rax+0x*]")
        .Find("call [rax+0x*]")
        .Entry(TEXT("/Script/ValetRuntime.Default__FortDagwoodVehicle"))
        .Find("call 0x*")
        .IsTarget("ret*")
        .Optional()
        .AtCall());

    static auto FortVehicleInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleInterface"));
    static auto Function = (UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortAthenaVehicle:RepairAllDamageableParts"));
    static auto bHasNewHealth = Function && Function->FindPropertyByName(FName(TEXT("NewHealth"))) != nullptr;

    auto VehicleInterface = HasAuthority() ? (IFortVehicleInterface*)GetInterfaceAddress(FortVehicleInterface) : nullptr;
    auto VehicleConfigs = VehicleInterface ? VehicleInterface->GetVehicleConfigs() : nullptr;

    if (!VehicleConfigs)
        return;

    auto& DamageableParts = GetDamageableParts();
    const auto& PartConfigs = VehicleConfigs->GetDamageableParts();
    const TArray<FVehicleDamageablePart> PrevDamageableParts = DamageableParts;

    for (auto& Part : DamageableParts)
    {
        if (Part.ConfigIndex >= 0 && Part.ConfigIndex < PartConfigs.Num())
            Part.Health = bHasNewHealth ? FMath::Min(NewHealth, PartConfigs[Part.ConfigIndex].MaxHealth) : PartConfigs[Part.ConfigIndex].MaxHealth;
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
            Part.Health = PartConfigs[ConfigIndex].GetbStartsDisabled() ? 0.0f : PartConfigs[ConfigIndex].MaxHealth;

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
