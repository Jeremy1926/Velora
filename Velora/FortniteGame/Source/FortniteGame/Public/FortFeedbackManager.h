#pragma once

#include "GameFramework/Actor.h"
#include "UObject/NameTypes.h"

class AFortPawn;
class UFortFeedbackBank;

enum class EFortFeedbackBroadcastFilter : uint8
{
    FFBF_Speaker,
    FFBF_SpeakerTeam,
    FFBF_SpeakerAdressee,
    FFBF_HumanPvP_Team1,
    FFBF_HumanPvP_Team2,
    FFBF_None_Max
};

struct FFortFeedbackHandle
{
    UFortFeedbackBank* FeedbackBank;
    FName EventName;
    bool bReadOnly;
    bool bBankDefined;
    EFortFeedbackBroadcastFilter BroadcastFilterOverride;
};

class AFortFeedbackManager : public AActor
{
public:
    static AFortFeedbackManager* GetFeedbackManager(UObject* WorldContextObject);

    void TriggerEvent(FFortFeedbackHandle Handle, AFortPawn* Instigator, AFortPawn* Recipient, float Delay, bool bOverriddenQueuing, bool bCanQue);
};
