#include "pch.h"
#include "FortFeedbackManager.h"
#include "Scanner.h"

AFortFeedbackManager* AFortFeedbackManager::GetFeedbackManager(UObject* WorldContextObject)
{
    static auto GetFeedbackManager = Anchor::Function<AFortFeedbackManager* (*)(UObject*)>(TEXT("AFortFeedbackManager::GetFeedbackManager"))
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
        .Get();

    TriggerEvent(this, Handle, Instigator, Recipient, Delay, bOverriddenQueuing, bCanQue);
}
