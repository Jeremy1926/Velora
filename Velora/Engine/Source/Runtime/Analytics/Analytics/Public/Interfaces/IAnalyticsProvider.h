#pragma once

#include "Containers/Array.h"
#include "AnalyticsEventAttribute.h"

class IAnalyticsProvider
{
public:
    bool StartSession()
    {
        return StartSession(TArray<FAnalyticsEventAttribute>());
    }

    virtual bool StartSession(const TArray<FAnalyticsEventAttribute>& Attributes) = 0;
};
