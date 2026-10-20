#include "pch.h"
#include "Athena/FortPlayerPawnAthena.h"
#include "Athena/FortPlayerControllerAthena.h"
#include "GameFramework/Controller.h"
#include "FortRegisteredPlayerInfo.h"
#include "Items/FortQuickBars.h"
#include "Items/FortPickup.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Templates/Casts.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"
#include "Detour.h"

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
    static auto bChecksPlayerPawnReady = []
    {
        auto UpdateAbilitySystemActor = Scanner::String(TEXT("UpdateAbilitySystemActor with no profile/world, clearing [%s]")).Start().Get();
        auto PlayerPawnReady = Scanner::Reference(UpdateAbilitySystemActor, "call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*; test rax, rax").Start();

        if (!PlayerPawnReady.Get())
            PlayerPawnReady = Scanner::Reference(UpdateAbilitySystemActor, "call 0x*; jmp 0x*; xor *, *; mov rcx, *; call 0x*; test *, *").Start();

        if (!PlayerPawnReady.Get())
            PlayerPawnReady = Scanner::CallTo(Scanner::String(TEXT("ExitAircraft: %s (Pawn %s)")).Start().Get()).Start();

        if (!PlayerPawnReady.Get())
            PlayerPawnReady = Scanner::Reference(UpdateAbilitySystemActor, "call 0x*; mov rcx, *; call 0x*; mov rax, [*]; mov rcx, *; call [rax+0x*]; test al, al").Start();

        if (!PlayerPawnReady.Has(std::format("cmp byte ptr [rcx+{:#x}], *", bIsPlayerPawnReady->GetOffset_ForInternal())))
            return false;

        THook(TEXT("UFortRegisteredPlayerInfo::PlayerPawnReadyUpdateAbilitySystemActor"), &UFortRegisteredPlayerInfo::PlayerPawnReadyUpdateAbilitySystemActor)
            .At(PlayerPawnReady)
            .FindCall(UpdateAbilitySystemActor)
            .AtCall();

        return true;
    }();

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
