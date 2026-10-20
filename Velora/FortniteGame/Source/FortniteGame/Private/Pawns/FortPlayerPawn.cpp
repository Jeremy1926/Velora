#include "pch.h"
#include "Pawns/FortPlayerPawn.h"
#include "Athena/Vehicles/FortAthenaVehicle.h"
#include "Items/FortPickup.h"
#include "Abilities/FortAbilitySystemComponent.h"
#include "Abilities/FortHealthSet.h"
#include "AI/FortAIPerceptionSystem.h"
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
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

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

AFortAthenaVehicle* AFortPlayerPawn::GetVehicle() const
{
    return Cast<AFortAthenaVehicle>(GetVehicleStateLocal().GetVehicle());
}

AFortAthenaVehicle* AFortPlayerPawn::ServerOnExitVehicle(ETryExitVehicleBehavior ExitForceBehavior)
{
    static auto bExitForceBehavior = ((UStruct*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerPawn:ServerOnExitVehicle")))->FindPropertyByName(FName(TEXT("ExitForceBehavior"))) != nullptr;
    static auto ServerOnExitVehicle = Scanner::Native(TEXT("/Script/FortniteGame.FortPlayerPawn:ServerOnExitVehicle")).Get(TEXT("AFortPlayerPawn::ServerOnExitVehicle"));

    if (bExitForceBehavior)
        return Scanner::Function<AFortAthenaVehicle* (*)(AFortPlayerPawn*, ETryExitVehicleBehavior)>(ServerOnExitVehicle)(this, ExitForceBehavior);

    return Scanner::Function<AFortAthenaVehicle* (*)(AFortPlayerPawn*)>(ServerOnExitVehicle)(this);
}

void AFortPlayerPawn::AddIgnoredActor(AActor* InActorToIgnore, const float InIgnoreDuration)
{
    static auto AddIgnoredActor = Scanner::Function<void (*)(AFortPlayerPawn*, AActor*, float)>(Scanner::Native(TEXT("/Script/FortniteGame.FortPlayerPawn:AddIgnoredActor")).Get(TEXT("AFortPlayerPawn::AddIgnoredActor")));

    AddIgnoredActor(this, InActorToIgnore, InIgnoreDuration);
}

const FVehiclePawnState& AFortPlayerPawn::GetVehicleStateLocal() const
{
    static auto VehicleStateLocal = GetClass()->FindPropertyByName(FName(TEXT("VehicleStateLocal")));

    return *VehicleStateLocal->ContainerPtrToValuePtr<FVehiclePawnState>(this);
}

void AFortPlayerPawn::OnBlueprintPlace()
{
    static auto OnBlueprintPlace = []
    {
        auto ClientBuild = Scanner::CallTo(Scanner::Reference(Scanner::String(TEXT("ServerCreateBuildingActor")).Find("lea rcx, [0x*]").Target(), "mov rdx, *").Start().Get()).Start();
        auto CheckAutoChangeMaterial = ClientBuild.Find("lea rdx, [rsp+0x*]; call 0x*; test al, al").Call();

        return (void (*)(AFortPlayerPawn*))(CheckAutoChangeMaterial.Get() ? CheckAutoChangeMaterial : ClientBuild).Find("mov rcx, [*+0x*]; test rcx, rcx; jz *; call 0x*").Call().Get(TEXT("AFortPlayerPawn::OnBlueprintPlace"));
    }();

    OnBlueprintPlace(this);
}

void AFortPlayerPawn::ServerHandlePickup_Implementation(AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
{
    if (Pickup && !Pickup->IsPendingKill())
        Pickup->SetPickupTarget(this, InFlyTime, InStartDirection, bPlayPickupSound);
}

void AFortPlayerPawn::ServerReviveFromDBNO_Implementation(AController* EventInstigator)
{
    auto FortAbilitySystemComponent = Cast<UFortAbilitySystemComponent>(GetAbilitySystemComponent());

    if (CanReviveFromDBNO(EventInstigator) && FortAbilitySystemComponent)
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

void AFortPlayerPawn::ServerSendZiplineState_Implementation(const FZiplinePawnState& InZiplineState)
{
    auto& ZiplineState = GetZiplineState();

    if (InZiplineState.GetAuthoritativeValue() > ZiplineState.GetAuthoritativeValue())
        memcpy(&ZiplineState, &InZiplineState, FZiplinePawnState::StaticStruct()->GetPropertiesSize());

    HandleZiplineStateChanged();
}

void AFortPlayerPawn::HandleZiplineStateChanged()
{
    static auto HandleZiplineStateChanged = Scanner::Function<void (*)(AFortPlayerPawn*)>(Scanner::CallTo(Scanner::Event(TEXT("ServerSendZiplineState")).Get()).Last("call 0x*").Target(TEXT("AFortPlayerPawn::HandleZiplineStateChanged")));

    HandleZiplineStateChanged(this);
}

FZiplinePawnState& AFortPlayerPawn::GetZiplineState()
{
    static auto ZiplineState = GetClass()->FindPropertyByName(FName(TEXT("ZiplineState")));

    return *ZiplineState->ContainerPtrToValuePtr<FZiplinePawnState>(this);
}

bool AFortPlayerPawn::CanReviveFromDBNO(AController* EventInstigator)
{
    static auto CanReviveFromDBNO = [this]
    {
        auto ResurrectHere = Scanner::String("ResurrectHere").Last("lea rax, [0x*]");

        if (Scanner::IsCode(ResurrectHere.Target()))
            return Scanner::At(ResurrectHere.Target(TEXT("AFortPlayerControllerZone::ResurrectHere"))).Find("call [*+0x*]").Slot(TEXT("AFortPlayerPawn::CanReviveFromDBNO"));

        auto VTable = *(const uint8_t***)this;
        auto Image = Scanner::Base();
        auto ImageEnd = Image + ((IMAGE_NT_HEADERS*)(Image + ((IMAGE_DOS_HEADER*)Image)->e_lfanew))->OptionalHeader.SizeOfImage;
        auto bIsDBNO = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bIsDBNO")));
        auto Offset = bIsDBNO->GetOffset_ForInternal() + bIsDBNO->GetByteOffset();
        auto Check = bIsDBNO->GetFieldMask() == 0xFF ? std::format("cmp byte ptr [rcx+{:#x}], 0x0; mov r*, rdx; mov r*, rcx", Offset) : std::format("test byte ptr [rcx+{:#x}], {:#x}; mov r*, rdx; mov r*, rcx", Offset, bIsDBNO->GetFieldMask());

        for (int32 Index = 0; Scanner::IsCode(VTable[Index]); Index++)
        {
            if (VTable[Index] >= Image && VTable[Index] < ImageEnd && Scanner::At(VTable[Index]).Has(Check))
                return Scanner::Slot(TEXT("AFortPlayerPawn::CanReviveFromDBNO"), Index * (int32)sizeof(void*));
        }

        return Scanner::Slot(TEXT("AFortPlayerPawn::CanReviveFromDBNO"), 0);
    }();

    return Scanner::Virtual<bool (*)(AFortPlayerPawn*, AController*)>(this, CanReviveFromDBNO)(this, EventInstigator);
}

void AFortPlayerPawn::UpdateDBNOInteractCollision()
{
    static auto UpdateDBNOInteractCollision = Scanner::Function<void (*)(AFortPlayerPawn*)>(Scanner::String(TEXT("DBNOInteractCollision")).Start().Get(TEXT("AFortPlayerPawn::UpdateDBNOInteractCollision")));

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

FGameplayTag& AFortPlayerPawn::GetEventReviveTag()
{
    static auto EventReviveTag = GetClass()->FindPropertyByName(FName(TEXT("EventReviveTag")));

    return *EventReviveTag->ContainerPtrToValuePtr<FGameplayTag>(this);
}
