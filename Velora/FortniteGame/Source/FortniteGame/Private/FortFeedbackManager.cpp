#include "pch.h"
#include "FortFeedbackManager.h"
#include "Scanner.h"

AFortFeedbackManager* AFortFeedbackManager::GetFeedbackManager(UObject* WorldContextObject)
{
    static auto GetFeedbackManager = (AFortFeedbackManager* (*)(UObject*))Scanner::Native(TEXT("/Script/FortniteGame.FortFeedbackManager:GetFeedbackManager")).Get(TEXT("AFortFeedbackManager::GetFeedbackManager"));

    return GetFeedbackManager(WorldContextObject);
}

void AFortFeedbackManager::TriggerEvent(FFortFeedbackHandle Handle, AFortPawn* Instigator, AFortPawn* Recipient, float Delay, bool bOverriddenQueuing, bool bCanQue)
{
    static auto TriggerEvent = (void (*)(AFortFeedbackManager*, FFortFeedbackHandle, AFortPawn*, AFortPawn*, float, bool, bool))Scanner::String(TEXT("Invalid EFortDayPhase::Type in AFortTimeOfDayManager::HandleDayPhaseChange.")).Find("mov byte ptr [rsp+0x52], 0x5; call 0x*").Call().Get(TEXT("AFortFeedbackManager::TriggerEvent"));

    TriggerEvent(this, Handle, Instigator, Recipient, Delay, bOverriddenQueuing, bCanQue);
}
