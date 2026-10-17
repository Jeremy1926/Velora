#pragma once

#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"

class IAnalyticsProvider;

enum class EAnalyticsProviderType
{
    Client,
    Server,
    ServerGameInstance
};

class UFortAnalytics
{
public:
    static TSharedPtr<IAnalyticsProvider> CreateAnalyticsProvider(const FString& UserID, const FString& GameSessionID, const FString& Platform, EAnalyticsProviderType ProviderType, const FString& GameState);
};
