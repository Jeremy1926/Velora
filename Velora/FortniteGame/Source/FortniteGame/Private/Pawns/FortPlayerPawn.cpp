#include "pch.h"
#include "Pawns/FortPlayerPawn.h"
#include "Athena/FortGameStateAthena.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Athena/Vehicles/FortVehicleInterface.h"
#include "Athena/Vehicles/FortVehicleSeatWeaponComponent.h"
#include "Items/FortPickup.h"
#include "Items/FortWeaponItemDefinition.h"
#include "Items/FortWorldItem.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Abilities/FortHealthSet.h"
#include "AI/FortAIPerceptionSystem.h"
#include "Player/FortPlayerController.h"
#include "Player/FortPlayerControllerZone.h"
#include "Player/StatManager.h"
#include "FortFeedbackManager.h"
#include "FortGameDeathPenalty.h"
#include "FortGameMode.h"
#include "FortGameState.h"
#include "GameplayEffect.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/WeakObjectPtr.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortPlayerPawn::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerPawn"));

    return Class;
}

UScriptStruct* FVehiclePawnState::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.VehiclePawnState"));

    return Struct;
}

AActor* FVehiclePawnState::GetVehicle() const
{
    static auto Vehicle = StaticStruct()->FindPropertyByName(FName(TEXT("Vehicle")));

    return *Vehicle->ContainerPtrToValuePtr<AActor*>(this);
}

uint8 FVehiclePawnState::GetSeatIndex() const
{
    static auto SeatIndex = StaticStruct()->FindPropertyByName(FName(TEXT("SeatIndex")));

    return *SeatIndex->ContainerPtrToValuePtr<uint8>(this);
}

FMulticastScriptDelegate* AFortPlayerPawn::GetOnVehicleSpawnedEvent()
{
    static auto OnVehicleSpawnedEvent = Anchor::Data<FMulticastScriptDelegate*>(TEXT("AFortPlayerPawn::OnVehicleSpawnedEvent"))
        .String(TEXT("&UFortDagwoodVehicleManager::OnVehicleSpawned"))
        .Optional()
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    return OnVehicleSpawnedEvent;
}

FMulticastScriptDelegate& AFortPlayerPawn::GetOnEnteredAircraft()
{
    static auto OnEnteredAircraft = StaticClass()->FindPropertyByName(FName(TEXT("OnEnteredAircraft")));

    return *OnEnteredAircraft->ContainerPtrToValuePtr<FMulticastScriptDelegate>(this);
}

AFortAthenaVehicle* AFortPlayerPawn::GetVehicle() const
{
    return Cast<AFortAthenaVehicle>(GetVehicleStateLocal().GetVehicle());
}

AActor* AFortPlayerPawn::GetHeldObject() const
{
    static auto HeldObject = GetClass()->FindPropertyByName(FName(TEXT("HeldObject")));

    return HeldObject ? HeldObject->ContainerPtrToValuePtr<TWeakObjectPtr<AActor>>(this)->Get() : nullptr;
}

void AFortPlayerPawn::SetHeldObject(AActor* InHeldObject)
{
    static auto HeldObject = GetClass()->FindPropertyByName(FName(TEXT("HeldObject")));

    *HeldObject->ContainerPtrToValuePtr<TWeakObjectPtr<AActor>>(this) = InHeldObject;
}

AFortAthenaVehicle* AFortPlayerPawn::ServerOnExitVehicle(ETryExitVehicleBehavior ExitForceBehavior)
{
    static auto bExitForceBehavior = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerPawn:ServerOnExitVehicle")))->FindPropertyByName(FName(TEXT("ExitForceBehavior"))) != nullptr;
    static auto ServerOnExitVehicle = Anchor::Function<const void*>(TEXT("AFortPlayerPawn::ServerOnExitVehicle"))
        .Native(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerOnExitVehicle"))
        .Get();

    if (bExitForceBehavior)
        return ((AFortAthenaVehicle* (*)(AFortPlayerPawn*, ETryExitVehicleBehavior))ServerOnExitVehicle)(this, ExitForceBehavior);

    return ((AFortAthenaVehicle* (*)(AFortPlayerPawn*))ServerOnExitVehicle)(this);
}

void AFortPlayerPawn::OnRep_InVehicle()
{
    DETOUR(AFortPlayerPawn, OnRep_InVehicle, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleSeatWeaponComponent:EquipVehicleWeapon")))
        .Body(TEXT("/Script/FortniteGame.FortPlayerPawn:OnRep_InVehicle"))
        .Inline());

    static auto FortVehicleInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortVehicleInterface"));

    auto& VehicleStateLocal = GetVehicleStateLocal();
    auto& VehicleStateRep = GetVehicleStateRep();
    auto Vehicle = VehicleStateLocal.GetVehicle();
    auto VehicleInterface = Vehicle ? (IFortVehicleInterface*)Vehicle->GetInterfaceAddress(FortVehicleInterface) : nullptr;

    if (VehicleInterface && (VehicleStateRep.GetVehicle() != Vehicle || VehicleStateRep.GetSeatIndex() != VehicleStateLocal.GetSeatIndex()))
    {
        if (auto WeaponComponent = VehicleInterface->GetSeatWeaponComponent(VehicleStateLocal.GetSeatIndex()))
            WeaponComponent->OnPawnExitSeat(VehicleInterface, this, VehicleStateLocal.GetSeatIndex());
    }

    Original();
}

FGuid* AFortPlayerPawn::GetLastEquippedWeaponOrGadgetGUID()
{
    static auto LastEquippedWeaponOrGadgetGUID = Anchor::Offset(TEXT("AFortPlayerPawn::LastEquippedWeaponOrGadgetGUID"))
        .If(::Anchor::Exists()
            .String(TEXT("UFortVehicleSeatWeaponComponent::UnEquipVehicleWeapon failed to cleanup WeaponSeatDefinition.VehicleGrantedWeaponItem properly. PawnValid:%d"))
            .Start()
            .Find("mov rdx, [*+0x*]; test rdx, rdx; jz 0x*; mov rax, [*]; mov rcx, *; call [rax+0x*]")
            .Find("call [rax+0x*]")
            .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"))
            .Is("push rbx; sub rsp, 0x20; mov rbx, rcx; test rdx, rdx; jz 0x*; mov rax, [rdx]; mov rcx, rdx; call [rax+0x*]; mov qword ptr [rbx+0x*], 0x0; add rsp, 0x20; pop rbx; ret")
            .Get())
        .Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerEquipLastWeaponOrGadget"))
        .Find("call [rax+*]", 2)
        .Entry(TEXT("/Script/FortniteGame.Default__FortPlayerPawnAthena"))
        .Find("call 0x*")
        .Target()
        .Find("or eax, [*+0x*]; or eax, [*+0x*]; or eax, [*+0x*]; jz 0x*; movups xmm0, [*+0x*]")
        .Find("movups xmm0, [*+0x*]")
        .Disp();

    return LastEquippedWeaponOrGadgetGUID ? (FGuid*)((uint8*)this + LastEquippedWeaponOrGadgetGUID) : nullptr;
}

void AFortPlayerPawn::SetLastEquippedWeaponOrGadget(UFortWorldItem* Item)
{
    static auto FortGadgetItemDefinition = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortGadgetItemDefinition"));

    auto LastEquippedWeaponOrGadgetGUID = GetLastEquippedWeaponOrGadgetGUID();
    auto ItemDefinition = Item ? Item->GetItemDefinitionBP() : nullptr;

    if (LastEquippedWeaponOrGadgetGUID && ItemDefinition && (ItemDefinition->IsA(UFortWeaponItemDefinition::StaticClass()) || (FortGadgetItemDefinition && ItemDefinition->IsA(FortGadgetItemDefinition))))
        *LastEquippedWeaponOrGadgetGUID = Item->GetItemGuid();
}

void AFortPlayerPawn::ServerEquipLastWeaponOrGadget_Implementation(bool bForce)
{
    static auto ServerEquipLastWeaponOrGadget = Anchor::VirtualFunction<void (*)(AFortPlayerPawn*, bool)>(TEXT("AFortPlayerPawn::ServerEquipLastWeaponOrGadget_Implementation"))
        .Exec(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerEquipLastWeaponOrGadget"))
        .Find("call [rax+*]", 2)
        .Get();

    ServerEquipLastWeaponOrGadget(this, bForce);
}

void AFortPlayerPawn::AddIgnoredActor(AActor* InActorToIgnore, const float InIgnoreDuration)
{
    static auto AddIgnoredActor = Anchor::Function<void (*)(AFortPlayerPawn*, AActor*, float)>(TEXT("AFortPlayerPawn::AddIgnoredActor"))
        .Native(TEXT("/Script/FortniteGame.FortPlayerPawn:AddIgnoredActor"))
        .Get();

    AddIgnoredActor(this, InActorToIgnore, InIgnoreDuration);
}

const FVehiclePawnState& AFortPlayerPawn::GetVehicleStateRep() const
{
    static auto VehicleStateRep = GetClass()->FindPropertyByName(FName(TEXT("VehicleStateRep")));

    return *VehicleStateRep->ContainerPtrToValuePtr<FVehiclePawnState>(this);
}

const FVehiclePawnState& AFortPlayerPawn::GetVehicleStateLocal() const
{
    static auto VehicleStateLocal = GetClass()->FindPropertyByName(FName(TEXT("VehicleStateLocal")));

    return *VehicleStateLocal->ContainerPtrToValuePtr<FVehiclePawnState>(this);
}

void AFortPlayerPawn::BeginSkydiving(bool bFromBus)
{
    static auto BeginSkydiving = Anchor::Function<void (*)(AFortPlayerPawn*, bool)>(TEXT("AFortPlayerPawn::BeginSkydiving"))
        .Native(TEXT("/Script/FortniteGame.FortPlayerPawn:BeginSkydiving"))
        .Get();

    BeginSkydiving(this, bFromBus);
}

void AFortPlayerPawn::OnBlueprintPlace()
{
    static auto OnBlueprintPlace = Anchor::Function<void (*)(AFortPlayerPawn*)>(TEXT("AFortPlayerPawn::OnBlueprintPlace"))
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .TryCall("lea rdx, [rsp+0x*]; call 0x*; test al, al")
        .Find("mov rcx, [*+0x*]; test rcx, rcx; jz *; call 0x*")
        .Call()
        .Or()
        .Event(TEXT("ServerCreateBuildingActor"))
        .Caller()
        .Find("lea r8, [rbp-0x*]; lea rdx, [rbp-0x*]; call 0x*; test al, al")
        .Find("call 0x*")
        .Target()
        .Find("mov rcx, [*+0x*]; test rcx, rcx; jz *; call 0x*")
        .Call()
        .Get();

    OnBlueprintPlace(this);
}

void AFortPlayerPawn::ServerHandlePickup_Implementation(AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
{
    if (Pickup && !Pickup->IsPendingKill())
        Pickup->SetPickupTarget(this, InFlyTime, InStartDirection, bPlayPickupSound);
}

void AFortPlayerPawn::ServerReviveFromDBNO_Implementation(AController* EventInstigator)
{
    if (CanReviveFromDBNO(EventInstigator))
        ForceReviveFromDBNO(EventInstigator);
}

void AFortPlayerPawn::ForceReviveFromDBNO(AController* EventInstigator)
{
    auto FortAbilitySystemComponent = Cast<UFortAbilitySystemComponent>(GetAbilitySystemComponent());

    if (FortAbilitySystemComponent)
    {
        const bool bSelfRevive = EventInstigator == GetController();
        auto ReviveGameplayEffect = bSelfRevive ? GetSelfReviveGameplayEffect() : GetTeammateReviveGameplayEffect();
        auto SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(ReviveGameplayEffect, 0.f, FortAbilitySystemComponent->MakeEffectContext());

        if (SpecHandle.IsValid())
        {
            SetIsDBNO(false);
            SetPlayedDying(false);

            const float ReviveHealth = GetSetByCallerReviveHealth().GetValueAtLevel(0.f);

            if (ReviveHealth > 0.f)
            {
                float CurrentHealth = 0.f;

                if (auto HealthSet = GetHealthSet())
                    CurrentHealth = HealthSet->GetHealth().GetCurrentValue();

                SpecHandle.Data->SetSetByCallerMagnitude(FName(TEXT("ReviveHeal")), ReviveHealth - CurrentHealth);
            }

            UpdateDBNOInteractCollision();

            GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

            static auto DBNOTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Action.Player.DBNO")));
            static auto DBNOAthenaTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Action.Player.DBNOAthena")));

            StopPerformingNativeAction(DBNOTag);
            StopPerformingNativeAction(DBNOAthenaTag);
            UFortAIPerceptionSystem::ForceForgetEnemy(this);

            FGameplayTagContainer AbilityTagsToCancel;
            AbilityTagsToCancel.AddTag(DBNOTag);
            AbilityTagsToCancel.AddTag(DBNOAthenaTag);
            FortAbilitySystemComponent->CancelAbilities(&AbilityTagsToCancel);

            FGameplayEventData Payload;
            Payload.Instigator = EventInstigator;
            Payload.Target = this;
            FortAbilitySystemComponent->HandleGameplayEvent(GetEventReviveTag(), &Payload);

            if (auto FeedbackManager = AFortFeedbackManager::GetFeedbackManager(this))
            {
                FFortFeedbackHandle Handle{ nullptr, FName(TEXT("Resurrected")), false, false, EFortFeedbackBroadcastFilter::FFBF_None_Max };

                FeedbackManager->TriggerEvent(Handle, EventInstigator ? Cast<AFortPawn>(EventInstigator->GetPawn()) : nullptr, this, -1.f, false, true);
            }

            auto World = GetWorld();
            auto PlayerController = Cast<AFortPlayerControllerZone>(GetController());

            if (World && PlayerController)
            {
                PlayerController->OnPawnRevived(EventInstigator);
                PlayerController->ResetQuickBarToPrimary();

                auto GameState = Cast<AFortGameState>(World->GetGameState());

                if (GameState && GameState->GetGameplayState() != EFortGameplayState::EndOfZone)
                {
                    if (auto GameMode = Cast<AFortGameMode>(World->GetAuthGameMode()))
                    {
                        if (auto DeathPenalty = GameMode->GetDeathPenaltyData().LoadSynchronous())
                            PlayerController->ApplyDeathDurabilityPenalty(bSelfRevive ? DeathPenalty->GetSelfResurrectDurabilityPenalty() : DeathPenalty->GetTeammateResurrectDurabilityPenalty());
                    }
                }
            }

            if (!bSelfRevive && EventInstigator)
            {
                auto InstigatorController = Cast<AFortPlayerControllerZone>(EventInstigator);

                if (InstigatorController && InstigatorController->GetStatManager())
                {
                    if (auto Stat = InstigatorController->GetStatManager()->AddOrGetStat(FName(TEXT("TeammateRevives")), EStatRecordingPeriod::Campaign))
                        Stat->ModifyStat(1, EStatMod::Delta);
                }
            }
        }
    }
}

UScriptStruct* FZiplinePawnState::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.ZiplinePawnState"));

    return Struct;
}

int32 FZiplinePawnState::GetAuthoritativeValue() const
{
    static auto AuthoritativeValue = StaticStruct()->FindPropertyByName(FName(TEXT("AuthoritativeValue")));

    return *AuthoritativeValue->ContainerPtrToValuePtr<int32>(this);
}

UScriptStruct* FFortClientObservedStatArray::StaticStruct()
{
    static auto Struct = (UScriptStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortClientObservedStatArray"));

    return Struct;
}

void FFortClientObservedStatArray::SetStatManager(UStatManager* InStatManager)
{
    static auto SetStatManager = Anchor::Function<void (*)(FFortClientObservedStatArray*, UStatManager*)>(TEXT("FFortClientObservedStatArray::SetStatManager"))
        .FieldPattern("48 8D ? %o 33 D2 E8", AFortPlayerPawn::StaticClass()->FindPropertyByName(FName(TEXT("ClientObservedStats"))))
        .Find("call 0x*")
        .Target()
        .Get();

    SetStatManager(this, InStatManager);
}

UStatManager* FFortClientObservedStatArray::GetMyStatManager() const
{
    static auto MyStatManager = StaticStruct()->FindPropertyByName(FName(TEXT("MyStatManager")));

    return *MyStatManager->ContainerPtrToValuePtr<UStatManager*>(this);
}

void AFortPlayerPawn::ServerSendZiplineState_Implementation(const FZiplinePawnState& InZiplineState)
{
    auto& ZiplineState = GetZiplineState();

    if (InZiplineState.GetAuthoritativeValue() > ZiplineState.GetAuthoritativeValue())
        memcpy(&ZiplineState, &InZiplineState, FZiplinePawnState::StaticStruct()->GetPropertiesSize());

    HandleZiplineStateChanged();
}

void AFortPlayerPawn::HandleZiplineStateChanged()
{
    static auto HandleZiplineStateChanged = Anchor::Function<void (*)(AFortPlayerPawn*)>(TEXT("AFortPlayerPawn::HandleZiplineStateChanged"))
        .Event(TEXT("ServerSendZiplineState"))
        .CallSite()
        .Last("call 0x*")
        .Target()
        .Get();

    HandleZiplineStateChanged(this);
}

FZiplinePawnState& AFortPlayerPawn::GetZiplineState()
{
    static auto ZiplineState = GetClass()->FindPropertyByName(FName(TEXT("ZiplineState")));

    return *ZiplineState->ContainerPtrToValuePtr<FZiplinePawnState>(this);
}

void AFortPlayerPawn::PlayGroupEmote(const UFortMontageItemDefinitionBase* EmoteAsset)
{
    if (auto PlayerController = Cast<AFortPlayerController>(GetController()))
        PlayerController->ServerPlayEmoteItem_Implementation(const_cast<UFortMontageItemDefinitionBase*>(EmoteAsset));
}

void AFortPlayerPawn::execPlayGroupEmote(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortPlayerPawn, execPlayGroupEmote, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortPlayerPawn:PlayGroupEmote"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortPlayerPawn:PlayGroupEmote"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortPlayerPawn:PlayGroupEmote")));

    UFortMontageItemDefinitionBase* EmoteAsset = nullptr;

    Stack.StepCompiledIn(&EmoteAsset);
    Stack.Code += !!Stack.Code;

    PlayGroupEmote(EmoteAsset);
}

void AFortPlayerPawn::UpdateGameStateModifiedGravity(float GravityMultiplier)
{
    auto GameState = Cast<AFortGameStateAthena>(GetWorld()->GetGameState());
    float NewGravityVal = 1.f;
    float NewVehicleGravityVal = 1.f;
    float NewJumpZ = 1.f;
    float NewJumpHorizontalAcceleration = 1.f;
    float NewJumpHorizontalVelocity = 1.f;

    if (GameState && GameState->AdjustedGravityMultiplier(this, NewGravityVal, NewVehicleGravityVal, NewJumpZ, NewJumpHorizontalAcceleration, NewJumpHorizontalVelocity))
        GravityMultiplier *= NewGravityVal;

    SetGravityMultiplier(GravityMultiplier);
}

void AFortPlayerPawn::execUpdateGameStateModifiedGravity(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortPlayerPawn, execUpdateGameStateModifiedGravity, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortPlayerPawn:UpdateGameStateModifiedGravity"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortPlayerPawn:UpdateGameStateModifiedGravity"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortPlayerPawn:UpdateGameStateModifiedGravity")));

    float GravityMultiplier = 0.f;

    Stack.StepCompiledIn(&GravityMultiplier);
    Stack.Code += !!Stack.Code;

    UpdateGameStateModifiedGravity(GravityMultiplier);
}

void AFortPlayerPawn::SetClientObservedStatsFromAnotherPawn(AFortPlayerPawn* OtherPawn)
{
    GetClientObservedStats().SetStatManager(OtherPawn ? OtherPawn->GetClientObservedStats().GetMyStatManager() : nullptr);
}

void AFortPlayerPawn::execSetClientObservedStatsFromAnotherPawn(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortPlayerPawn, execSetClientObservedStatsFromAnotherPawn, Anchor::Hook()
        .Native(TEXT("/Script/FortniteGame.FortPlayerPawn:SetClientObservedStatsFromAnotherPawn"))
        .Is("ret*")
        .Or()
        .StubExec(TEXT("/Script/FortniteGame.FortPlayerPawn:SetClientObservedStatsFromAnotherPawn"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortPlayerPawn:SetClientObservedStatsFromAnotherPawn")));

    AFortPlayerPawn* OtherPawn = nullptr;

    Stack.StepCompiledIn(&OtherPawn);
    Stack.Code += !!Stack.Code;

    SetClientObservedStatsFromAnotherPawn(OtherPawn);
}

TArray<const UAthenaGliderItemDefinition*>& AFortPlayerPawn::GetGliderOverrideStack()
{
    static auto GliderOverrideStack = StaticClass()->FindPropertyByName(FName(TEXT("GliderOverrideStack")));

    return *GliderOverrideStack->ContainerPtrToValuePtr<TArray<const UAthenaGliderItemDefinition*>>(this);
}

void AFortPlayerPawn::PushGliderOverride(const UAthenaGliderItemDefinition* InGliderOverride)
{
    if (InGliderOverride)
        GetGliderOverrideStack().Add(InGliderOverride);
}

void AFortPlayerPawn::execPushGliderOverride(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortPlayerPawn, execPushGliderOverride, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerPawn:PushGliderOverride")))
        .Native(TEXT("/Script/FortniteGame.FortPlayerPawn:PushGliderOverride"))
        .Is("ret*")
        .Or()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerPawn:PushGliderOverride")))
        .StubExec(TEXT("/Script/FortniteGame.FortPlayerPawn:PushGliderOverride"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortPlayerPawn:PushGliderOverride")));

    UAthenaGliderItemDefinition* InGliderOverride = nullptr;

    Stack.StepCompiledIn(&InGliderOverride);
    Stack.Code += !!Stack.Code;

    PushGliderOverride(InGliderOverride);
}

void AFortPlayerPawn::PopGliderOverride(const UAthenaGliderItemDefinition* InGliderToRemove)
{
    auto& GliderOverrideStack = GetGliderOverrideStack();

    for (int32 Index = GliderOverrideStack.Num() - 1; Index >= 0; Index--)
    {
        if (GliderOverrideStack[Index] != InGliderToRemove)
            continue;

        GliderOverrideStack.RemoveAt(Index);
        return;
    }
}

void AFortPlayerPawn::execPopGliderOverride(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortPlayerPawn, execPopGliderOverride, Anchor::Hook()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerPawn:PopGliderOverride")))
        .Native(TEXT("/Script/FortniteGame.FortPlayerPawn:PopGliderOverride"))
        .Is("ret*")
        .Or()
        .If(StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerPawn:PopGliderOverride")))
        .StubExec(TEXT("/Script/FortniteGame.FortPlayerPawn:PopGliderOverride"))
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortPlayerPawn:PopGliderOverride")));

    UAthenaGliderItemDefinition* InGliderToRemove = nullptr;

    Stack.StepCompiledIn(&InGliderToRemove);
    Stack.Code += !!Stack.Code;

    PopGliderOverride(InGliderToRemove);
}

bool AFortPlayerPawn::CanReviveFromDBNO(AController* EventInstigator)
{
    static auto CanReviveFromDBNO = Anchor::VirtualFunction<bool (*)(AFortPlayerPawn*, AController*)>(TEXT("AFortPlayerPawn::CanReviveFromDBNO"))
        .String("ResurrectHere")
        .Last("lea rax, [0x*]")
        .Target()
        .IsCode()
        .Log(TEXT("AFortPlayerControllerZone::ResurrectHere"))
        .Find("call [*+0x*]")
        .Or()
        .BoolTestIn(this, (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bIsDBNO"))), "; mov r*, rdx; mov r*, rcx")
        .Get();

    return CanReviveFromDBNO(this, EventInstigator);
}

void AFortPlayerPawn::UpdateDBNOInteractCollision()
{
    static auto UpdateDBNOInteractCollision = Anchor::Function<void (*)(AFortPlayerPawn*)>(TEXT("AFortPlayerPawn::UpdateDBNOInteractCollision"))
        .String(TEXT("DBNOInteractCollision"))
        .Start()
        .Get();

    UpdateDBNOInteractCollision(this);
}

TSubclassOf<UGameplayEffect>& AFortPlayerPawn::GetSelfReviveGameplayEffect()
{
    static auto SelfReviveGameplayEffect = GetClass()->FindPropertyByName(FName(TEXT("SelfReviveGameplayEffect")));

    return *SelfReviveGameplayEffect->ContainerPtrToValuePtr<TSubclassOf<UGameplayEffect>>(this);
}

TSubclassOf<UGameplayEffect>& AFortPlayerPawn::GetTeammateReviveGameplayEffect()
{
    static auto TeammateReviveGameplayEffect = GetClass()->FindPropertyByName(FName(TEXT("TeammateReviveGameplayEffect")));

    return *TeammateReviveGameplayEffect->ContainerPtrToValuePtr<TSubclassOf<UGameplayEffect>>(this);
}

FScalableFloat& AFortPlayerPawn::GetSetByCallerReviveHealth()
{
    static auto SetByCallerReviveHealth = GetClass()->FindPropertyByName(FName(TEXT("SetByCallerReviveHealth")));

    return *SetByCallerReviveHealth->ContainerPtrToValuePtr<FScalableFloat>(this);
}

FGameplayTag AFortPlayerPawn::GetEventReviveTag()
{
    static auto EventReviveTag = GetClass()->FindPropertyByName(FName(TEXT("EventReviveTag")));

    return EventReviveTag->GetPropertyValue_InContainer<FGameplayTag>(this);
}

FFortClientObservedStatArray& AFortPlayerPawn::GetClientObservedStats()
{
    static auto ClientObservedStats = GetClass()->FindPropertyByName(FName(TEXT("ClientObservedStats")));

    return *ClientObservedStats->ContainerPtrToValuePtr<FFortClientObservedStatArray>(this);
}

APawn* AFortPlayerPawn::GetTetherPawn() const
{
    static auto TetherComponentProperty = GetClass()->FindPropertyByName(FName(TEXT("TetherComponent")));

    auto TetherComponent = *TetherComponentProperty->ContainerPtrToValuePtr<UObject*>(this);

    if (!TetherComponent)
        return nullptr;

    static auto TetherPawn = TetherComponent->GetClass()->FindPropertyByName(FName(TEXT("TetherPawn")));

    return *TetherPawn->ContainerPtrToValuePtr<APawn*>(TetherComponent);
}
