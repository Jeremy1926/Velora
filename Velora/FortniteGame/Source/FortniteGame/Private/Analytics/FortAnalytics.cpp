#include "pch.h"
#include "Analytics/FortAnalytics.h"
#include "Scanner.h"

TSharedPtr<IAnalyticsProvider> UFortAnalytics::CreateAnalyticsProvider(const FString& UserID, const FString& GameSessionID, const FString& Platform, EAnalyticsProviderType ProviderType, const FString& GameState)
{
    static auto CreateAnalyticsProvider = (TSharedPtr<IAnalyticsProvider> (*)(const FString&, const FString&, const FString&, EAnalyticsProviderType, const FString&))Scanner::String(TEXT("TeamChatroom")).Start().Find("call 0x*; add r*, 0x???*").Target(TEXT("UFortAnalytics::CreateAnalyticsProvider"));

    return CreateAnalyticsProvider(UserID, GameSessionID, Platform, ProviderType, GameState);
}

void UFortAnalytics::FireEvent_BuildingAction(AFortPlayerController* PlayerController, const TCHAR* ActionType, ABuildingActor* BuildingActor, int32 ResourcesUsed)
{
    static auto FireEvent_BuildingAction = (void (*)(AFortPlayerController*, const TCHAR*, ABuildingActor*, int32))Scanner::String(TEXT("Building.BuildingAction")).Start().Get(TEXT("UFortAnalytics::FireEvent_BuildingAction"));

    FireEvent_BuildingAction(PlayerController, ActionType, BuildingActor, ResourcesUsed);
}
