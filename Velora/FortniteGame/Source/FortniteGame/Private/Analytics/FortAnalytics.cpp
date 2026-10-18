#include "pch.h"
#include "Analytics/FortAnalytics.h"
#include "Scanner.h"

TSharedPtr<IAnalyticsProvider> UFortAnalytics::CreateAnalyticsProvider(const FString& UserID, const FString& GameSessionID, const FString& Platform, EAnalyticsProviderType ProviderType, const FString& GameState)
{
    static auto CreateAnalyticsProvider = (TSharedPtr<IAnalyticsProvider> (*)(const FString&, const FString&, const FString&, EAnalyticsProviderType, const FString&))Scanner::String(TEXT("TeamChatroom")).Start().Find("call 0x*; add r*, 0x???*").Target();

    return CreateAnalyticsProvider(UserID, GameSessionID, Platform, ProviderType, GameState);
}
