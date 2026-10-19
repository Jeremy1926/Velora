#pragma once

#include "Engine/GameInstance.h"
#include "Templates/SharedPointer.h"

class IAnalyticsProvider;

class UFortGameInstance : public UGameInstance
{
public:
    TSharedPtr<IAnalyticsProvider>& GetServerAnalyticsProvider();

    TSharedPtr<IAnalyticsProvider>* GetServerEsportsAnalyticsProvider();
};
