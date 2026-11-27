#pragma once

#include "Delegates/Delegate.h"
#include "Engine/EngineTypes.h"
#include "Templates/Function.h"
#include "UObject/ScriptDelegates.h"

typedef TDelegate<void()> FTimerDelegate;

struct FTimerUnifiedDelegate
{
    explicit FTimerUnifiedDelegate(FTimerDelegate const& D)
        : FuncDelegate(D)
    {
    }

    FTimerDelegate FuncDelegate;
    FScriptDelegate FuncDynDelegate;
    TFunction<void()> FuncCallback;
};

struct alignas(16) FTimerDelegateVariant
{
    FTimerDelegateVariant(FTimerDelegate const& D, int32 TypeIndexOffset)
        : FuncDelegate(D)
    {
        Storage[TypeIndexOffset - sizeof(FTimerDelegate)] = 1;
    }

    FTimerDelegate FuncDelegate;
    uint8 Storage[sizeof(TFunction<void()>)] = {};
};

struct FTimerManagerTimerParameters
{
    bool bLoop = false;
    bool bMaxOncePerFrame = false;
    float FirstDelay = -1.f;
};

class FTimerManager
{
public:
    template<class UserClass>
    void SetTimer(FTimerHandle& InOutHandle, UserClass* InObj, void (UserClass::*InTimerMethod)(), float InRate, bool InbLoop = false, float InFirstDelay = -1.f)
    {
        InternalSetTimer(InOutHandle, FTimerUnifiedDelegate(FTimerDelegate::CreateUObject(InObj, InTimerMethod)), InRate, InbLoop, InFirstDelay);
    }

    void SetTimer(FTimerHandle& InOutHandle, FTimerDelegate const& InDelegate, float InRate, bool InbLoop, float InFirstDelay = -1.f)
    {
        InternalSetTimer(InOutHandle, FTimerUnifiedDelegate(InDelegate), InRate, InbLoop, InFirstDelay);
    }

    static bool CanSetTimers();

private:
    void InternalSetTimer(FTimerHandle& InOutHandle, FTimerUnifiedDelegate&& InDelegate, float InRate, bool InbLoop, float InFirstDelay);
};
