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
