#pragma once

#include "Containers/Array.h"

struct FAnalyticsEventAttribute;

class IAnalyticsProvider
{
public:
    bool StartSession()
    {
        return StartSession(TArray<FAnalyticsEventAttribute>());
    }

    virtual bool StartSession(const TArray<FAnalyticsEventAttribute>& Attributes) = 0;
};
