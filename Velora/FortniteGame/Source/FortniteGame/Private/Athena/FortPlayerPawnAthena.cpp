#include "pch.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "Athena/FortPlayerStateAthena.h"
#include "GameFramework/Controller.h"
#include "GameplayTagAssetInterface.h"
#include "FortRegisteredPlayerInfo.h"
#include "Items/FortQuickBars.h"
#include "Items/FortPickup.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AFortPlayerPawnAthena::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlayerPawnAthena"));

    return Class;
}

void AFortPlayerPawnAthena::OnPlayerPawnReady()
{
    DETOUR(AFortPlayerPawnAthena, OnPlayerPawnReady, Anchor::Hook()
        .String(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]"))
        .Start()
        .Reference("call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; test rax, rax")
        .Start()
        .Or()
        .String(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]"))
        .Start()
        .Reference("call 0x*; jmp 0x*; xor *, *; mov rcx, *; call 0x*; test *, *")
        .Start()
        .Or()
        .String(TEXT("ExitAircraft: %s (Pawn %s)"))
        .Caller()
        .Or()
        .String(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]"))
        .Start()
        .Reference("call 0x*; mov rcx, *; call 0x*; mov rax, [*]; mov rcx, *; call [rax+0x*]; test al, al")
        .Start()
        .Inline());

    static auto bIsPlayerPawnReady = (UBoolProperty*)GetClass()->FindPropertyByName(FName(TEXT("bIsPlayerPawnReady")));
    static auto PlayerPawnReady = Anchor::Address()
        .String(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]"))
        .Start()
        .Reference("call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; test rax, rax")
        .Start()
        .Or()
        .String(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]"))
        .Start()
        .Reference("call 0x*; jmp 0x*; xor *, *; mov rcx, *; call 0x*; test *, *")
        .Start()
        .Or()
        .String(TEXT("ExitAircraft: %s (Pawn %s)"))
        .Caller()
        .Or()
        .String(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]"))
        .Start()
        .Reference("call 0x*; mov rcx, *; call 0x*; mov rax, [*]; mov rcx, *; call [rax+0x*]; test al, al")
        .Start()
        .Get();

    static auto bChecksPlayerPawnReady = Anchor::Exists()
        .At(PlayerPawnReady)
        .FieldHas("cmp byte ptr [rcx+%x], *", bIsPlayerPawnReady)
        .Get();

    static auto PlayerPawnReadyUpdateAbilitySystemActor = THook(TEXT("UFortRegisteredPlayerInfo::PlayerPawnReadyUpdateAbilitySystemActor"), &UFortRegisteredPlayerInfo::PlayerPawnReadyUpdateAbilitySystemActor)
        .If(bChecksPlayerPawnReady)
        .At(PlayerPawnReady)
        .FindCallToString(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]"))
        .AtCall();

    auto PlayerController = Cast<AFortPlayerControllerAthena>(GetController());
    auto bWasPlayerPawnReady = bIsPlayerPawnReady->GetPropertyValue_InContainer(this);

    if (PlayerController && !bChecksPlayerPawnReady)
    {
        if (auto MyPlayerInfo = PlayerController->GetMyPlayerInfo())
            MyPlayerInfo->UpdateAbilitySystemActor(false);
    }

    Original();

    if (bChecksPlayerPawnReady && (bWasPlayerPawnReady || !bIsPlayerPawnReady->GetPropertyValue_InContainer(this)))
        return;

    if (PlayerController)
    {
        if (auto QuickBars = PlayerController->GetQuickBars())
        {
            QuickBars->ActivateSlot(EFortQuickBars::Primary, 0, 0.f, true);

            auto& Slots = QuickBars->GetQuickBar(EFortQuickBars::Primary).Slots;

            if (Slots.Num() && Slots[0].Items.Num())
                PlayerController->ClientExecuteInventoryItem(Slots[0].Items[0], 0.0f, false, true);
        }
    }
}

void AFortPlayerPawnAthena::ServerHandlePickup_Implementation(AFortPickup* Pickup, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
{
    AFortPlayerPawn::ServerHandlePickup_Implementation(Pickup, InFlyTime / GetPickupSpeedMultiplier(), InStartDirection, bPlayPickupSound);
}

void AFortPlayerPawnAthena::ServerHandlePickupWithSwap_Implementation(AFortPickup* Pickup, const FGuid& Swap, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
{
    ServerHandlePickup_Implementation(Pickup, InFlyTime, InStartDirection, bPlayPickupSound);

    if (Pickup && !Pickup->IsPendingKill() && Pickup->GetPickupLocationData().PickupTarget == this)
        Pickup->SetSwapItemGuid(Swap);
}

void AFortPlayerPawnAthena::ServerHandlePickupWithRequestedSwap_Implementation(AFortPickup* Pickup, const FGuid& Swap, float InFlyTime, const FVector& InStartDirection, bool bPlayPickupSound)
{
    ServerHandlePickupWithSwap_Implementation(Pickup, Swap, InFlyTime, InStartDirection, bPlayPickupSound);
}

void AFortPlayerPawnAthena::ServerHandlePickupInfo_Implementation(AFortPickup* Pickup, const FFortPickupRequestInfo& Params)
{
    if (Params.bUseRequestedSwap)
        return ServerHandlePickupWithRequestedSwap_Implementation(Pickup, Params.SwapWithItem, Params.FlyTime, Params.Direction, Params.bPlayPickupSound);

    ServerHandlePickup_Implementation(Pickup, Params.FlyTime, Params.Direction, Params.bPlayPickupSound);
}

void AFortPlayerPawnAthena::ServerReviveFromDBNO_Implementation(AController* EventInstigator)
{
    AFortPlayerPawn::ServerReviveFromDBNO_Implementation(EventInstigator);

    if (!GetIsDBNO())
    {
        GetDBNORevivalStacking() = 0;

        static auto DBNOBeingRevivedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Status.DBNOBeingRevived")));
        static auto BeingRevivedFromDBNOCueTag = FGameplayTag::RequestGameplayTag(FName(TEXT("GameplayCue.Athena.Player.BeingRevivedFromDBNO")));

        auto AbilitySystemComponent = GetAbilitySystemComponent();
        AbilitySystemComponent->RemoveLooseGameplayTag(DBNOBeingRevivedTag);
        AbilitySystemComponent->RemoveGameplayCue(BeingRevivedFromDBNOCueTag);
    }
}

void AFortPlayerPawnAthena::ForceReviveFromDBNO(AController* EventInstigator)
{
    AFortPlayerPawn::ForceReviveFromDBNO(EventInstigator);

    if (!GetIsDBNO())
    {
        GetDBNORevivalStacking() = 0;

        static auto DBNOBeingRevivedTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Gameplay.Status.DBNOBeingRevived")));
        static auto BeingRevivedFromDBNOCueTag = FGameplayTag::RequestGameplayTag(FName(TEXT("GameplayCue.Athena.Player.BeingRevivedFromDBNO")));

        auto AbilitySystemComponent = GetAbilitySystemComponent();
        AbilitySystemComponent->RemoveLooseGameplayTag(DBNOBeingRevivedTag);
        AbilitySystemComponent->RemoveGameplayCue(BeingRevivedFromDBNOCueTag);
    }
}

void AFortPlayerPawnAthena::execForceReviveFromDBNO(FFrame& Stack, void* const Z_Param__Result)
{
    DETOUR(AFortPlayerPawnAthena, execForceReviveFromDBNO, Anchor::Hook()
        .Body(TEXT("/Script/FortniteGame.FortPlayerPawnAthena:ForceReviveFromDBNO"))
        .Is("ret*")
        .Optional()
        .InExec(TEXT("/Script/FortniteGame.FortPlayerPawnAthena:ForceReviveFromDBNO")));

    Stack.Code += !!Stack.Code;

    if (GetIsDBNO())
        ForceReviveFromDBNO(GetController());
}

EDeathCause AFortPlayerPawnAthena::ToDeathCause(const FGameplayTagContainer& InTags, bool bWasDBNO)
{
    static auto ToDeathCause = Anchor::Function<EDeathCause (*)(AFortPlayerPawnAthena*, const FGameplayTagContainer&, bool)>(TEXT("AFortPlayerPawnAthena::ToDeathCause"))
        .Event(TEXT("ClientReportDBNO"))
        .CallSite()
        .Start()
        .Find("cmov* rsi, r*; xor r8d, r8d; call 0x*")
        .Call()
        .Get();

    return ToDeathCause(this, InTags, bWasDBNO);
}

void AFortPlayerPawnAthena::InitializeDeathInfoOnPawnDeath(const FGameplayTagContainer& InTags, AController* EventInstigator, AController* DBNOFinisher, AActor* DamageCauser)
{
    static auto bWasDBNOOnDeath = AFortPawn::StaticClass()->FindPropertyByName(FName(TEXT("bWasDBNOOnDeath")));

    static auto bPawnToDeathCause = Anchor::Exists()
        .Event(TEXT("ClientReportDBNO"))
        .CallSite()
        .Start()
        .Find("cmov* rsi, r*; xor r8d, r8d; call 0x*")
        .Get();

    static auto bToDeathCause = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortPlayerStateAthena:ToDeathCause"))
        .Or()
        .Event(TEXT("ClientReportDBNO"))
        .CallSite()
        .Start()
        .Find("cmov* rsi, r*")
        .Find("call 0x*")
        .LastSince("xor edx, edx", "cmov* rsi, r*")
        .Get();

    static auto GameplayTagAssetInterface = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/GameplayTags.GameplayTagAssetInterface"));

    auto PlayerState = Cast<AFortPlayerStateAthena>(GetPlayerState());

    if (!PlayerState)
        return;

    auto bWasDBNO = bWasDBNOOnDeath ? GetWasDBNOOnDeath() : GetIsDBNO();
    auto Finisher = DBNOFinisher ? DBNOFinisher : EventInstigator;
    auto FinisherPawn = Finisher ? Cast<AFortPawn>(Finisher->GetPawn()) : nullptr;

    FDeathInfo DeathInfo;

    DeathInfo.GetFinisherOrDowner() = EventInstigator ? Cast<AFortPlayerStateAthena>(Finisher->GetPlayerState()) : PlayerState;
    DeathInfo.SetDBNO(false);

    if (bPawnToDeathCause)
        DeathInfo.GetDeathCause() = ToDeathCause(InTags, bWasDBNO);
    else if (bToDeathCause)
        DeathInfo.GetDeathCause() = AFortPlayerStateAthena::ToDeathCause(InTags, bWasDBNO);

    if (auto Downer = DeathInfo.GetDowner(); Downer && bWasDBNO)
        *Downer = *PlayerState->GetDeathInfo().GetDowner();

    if (auto Distance = DeathInfo.GetDistance(); Distance && FinisherPawn)
        *Distance = FinisherPawn->GetDistanceTo(this);

    if (auto DeathLocation = DeathInfo.GetDeathLocation())
        *DeathLocation = GetActorLocation();

    if (auto DeathTags = DeathInfo.GetDeathTags())
        *DeathTags = InTags;

    auto FinisherAbilitySystemComponent = FinisherPawn ? FinisherPawn->GetAbilitySystemComponent() : nullptr;

    if (auto FinisherOrDownerTags = DeathInfo.GetFinisherOrDownerTags(); FinisherOrDownerTags && FinisherAbilitySystemComponent)
        ((IGameplayTagAssetInterface*)FinisherAbilitySystemComponent->GetInterfaceAddress(GameplayTagAssetInterface))->GetOwnedGameplayTags(*FinisherOrDownerTags);

    if (auto VictimTags = DeathInfo.GetVictimTags(); VictimTags && GetAbilitySystemComponent())
        ((IGameplayTagAssetInterface*)GetAbilitySystemComponent()->GetInterfaceAddress(GameplayTagAssetInterface))->GetOwnedGameplayTags(*VictimTags);

    PlayerState->InitializeDeathInfo(DeathInfo);
}

float AFortPlayerPawnAthena::GetPickupSpeedMultiplier() const
{
    static auto PickupSpeedMultiplier = GetClass()->FindPropertyByName(FName(TEXT("PickupSpeedMultiplier")));

    return *PickupSpeedMultiplier->ContainerPtrToValuePtr<float>(this);
}

uint8& AFortPlayerPawnAthena::GetDBNORevivalStacking()
{
    static auto DBNORevivalStacking = GetClass()->FindPropertyByName(FName(TEXT("DBNORevivalStacking")));

    return *DBNORevivalStacking->ContainerPtrToValuePtr<uint8>(this);
}
