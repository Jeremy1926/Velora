#pragma once

#include "HAL/Platform.h"
#include "CoreGlobals.h"
#include "Delegates/Delegate.h"

template <class T>
class TConsoleVariableData
{
public:
    T GetValueOnGameThread() const
    {
        return ShadowedValue[0];
    }

    T GetValueOnAnyThread(bool bForceGameThread = false) const
    {
        return ShadowedValue[GetShadowIndex(bForceGameThread)];
    }

private:
    T ShadowedValue[2];

    static uint32 GetShadowIndex(bool bForceGameThread = false)
    {
        if (bForceGameThread)
        {
            return 0;
        }

        return IsInGameThread() ? 0 : 1;
    }
};

enum EConsoleVariableFlags
{
    ECVF_SetByCode = 0x8000000
};

class IConsoleVariable;

DECLARE_DELEGATE_OneParam(FConsoleVariableDelegate, IConsoleVariable*);

class IConsoleObject
{
public:
    TConsoleVariableData<int32>* AsVariableInt();
};

class IConsoleVariable : public IConsoleObject
{
public:
    void Set(const TCHAR* InValue, EConsoleVariableFlags SetBy);

    void Set(int32 InValue, EConsoleVariableFlags SetBy);

    void SetOnChangedCallback(const FConsoleVariableDelegate& Callback);

    int32 GetInt() const;

    float GetFloat() const;
};

class IConsoleManager
{
public:
    IConsoleVariable* FindConsoleVariable(const TCHAR* Name) const;

    TConsoleVariableData<int32>* FindTConsoleVariableDataInt(const TCHAR* Name) const
    {
        IConsoleVariable* P = FindConsoleVariable(Name);

        return P ? P->AsVariableInt() : 0;
    }

    FORCEINLINE static IConsoleManager& Get()
    {
        if (!Singleton)
        {
            SetupSingleton();
        }

        return *Singleton;
    }

private:
    static IConsoleManager*& Singleton;

    static void SetupSingleton();
};
