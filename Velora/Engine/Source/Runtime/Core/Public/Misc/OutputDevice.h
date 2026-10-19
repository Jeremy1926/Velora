#pragma once

#include "HAL/Platform.h"
#include "Logging/LogVerbosity.h"

class FArchive;
class FName;

class FOutputDevice
{
public:
    FOutputDevice()
        : bSuppressEventTag(false)
        , bAutoEmitLineTerminator(true)
    {
    }

    virtual ~FOutputDevice()
    {
    }

    virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) = 0;

    virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category, const double Time)
    {
        Serialize(V, Verbosity, Category);
    }

    virtual void Flush()
    {
    }

    virtual void TearDown()
    {
    }

    virtual void Dump(FArchive& Ar)
    {
    }

    virtual bool IsMemoryOnly() const
    {
        return false;
    }

    virtual bool CanBeUsedOnAnyThread() const
    {
        return false;
    }

    virtual bool CanBeUsedOnMultipleThreads() const
    {
        return false;
    }

protected:
    bool bSuppressEventTag;
    bool bAutoEmitLineTerminator;
};
