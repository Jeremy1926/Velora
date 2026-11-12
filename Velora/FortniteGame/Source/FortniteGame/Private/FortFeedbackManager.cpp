#include "pch.h"
#include "FortFeedbackManager.h"
#include "Athena/FortGameStateAthena.h"
#include "Engine/World.h"
#include "Templates/Casts.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

AFortFeedbackManager* AFortFeedbackManager::GetFeedbackManager(UObject* WorldContextObject)
{
    static auto bInlined = Anchor::Exists()
        .Exec(TEXT("/Script/FortniteGame.FortFeedbackManager:GetFeedbackManager"))
        .Find("setnz *; add r*, r*")
        .Find("call [rax+0x*]; test rax, rax; jz 0x*; mov rcx, rax; call 0x*; test rax, rax; jz 0x*; mov rax, [rax+0x*]")
        .Get();

    if (bInlined)
    {
        static auto FeedbackManager = AFortGameStateAthena::StaticClass()->FindPropertyByName(FName(TEXT("FeedbackManager")));

        auto World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
        auto GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;

        return GameState && FeedbackManager ? *FeedbackManager->ContainerPtrToValuePtr<AFortFeedbackManager*>(GameState) : nullptr;
    }

    static auto GetFeedbackManager =Anchor::Function<AFortFeedbackManager* (*)(UObject*)>(TEXT("AFortFeedbackManager::GetFeedbackManager"))
        .Native(TEXT("/Script/FortniteGame.FortFeedbackManager:GetFeedbackManager"))
        .Get();

    return GetFeedbackManager(WorldContextObject);
}

void AFortFeedbackManager::TriggerEvent(FFortFeedbackHandle Handle, AFortPawn* Instigator, AFortPawn* Recipient, float Delay, bool bOverriddenQueuing, bool bCanQue)
{
    static auto TriggerEvent = Anchor::Function<void (*)(AFortFeedbackManager*, FFortFeedbackHandle, AFortPawn*, AFortPawn*, float, bool, bool)>(TEXT("AFortFeedbackManager::TriggerEvent"))
        .String(TEXT("Invalid EFortDayPhase::Type in AFortTimeOfDayManager::HandleDayPhaseChange."))
        .Find("mov byte ptr [rsp+0x*], 0x5; call 0x*")
        .Call()
        .Or()
        .Native(TEXT("/Script/FortniteGame.FortKismetLibrary:TriggerFeedbackEvent"))
        .Call(3)
        .Or()
        .Exec(TEXT("/Script/FortniteGame.FortKismetLibrary:TriggerFeedbackEvent"))
        .Find("add [*+0x20], *")
        .Find("mov rdx, rax; movss [rsp+0x20], xmm*; call 0x*")
        .Call()
        .Call(4)
        .Get();

    if (FName::IsOutlineNumber())
        ((void (*)(AFortFeedbackManager*, FFortFeedbackHandle::FOutlineNumberLayout, AFortPawn*, AFortPawn*, float, bool, bool))TriggerEvent)(this, Handle.ToOutlineNumberLayout(), Instigator, Recipient, Delay, bOverriddenQueuing, bCanQue);
    else
        TriggerEvent(this, Handle, Instigator, Recipient, Delay, bOverriddenQueuing, bCanQue);
}
