#pragma once

#include "UObject/Object.h"
#include "Delegates/Delegate.h"

enum class EUpdateCompletionStatus : uint8
{
    UpdateUnknown,
    UpdateSuccess,
    UpdateSuccess_NoChange,
    UpdateSuccess_NeedsReload,
    UpdateSuccess_NeedsRelaunch,
    UpdateSuccess_NeedsPatch,
    UpdateFailure_PatchCheck,
    UpdateFailure_HotfixCheck,
    UpdateFailure_NotLoggedIn,
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateCheckComplete, EUpdateCompletionStatus);

class UUpdateManager : public UObject
{
public:
    FOnUpdateCheckComplete& OnUpdateCheckComplete();
};
