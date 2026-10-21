#include "pch.h"
#include "Analytics/FortAnalytics.h"
#include "Scanner.h"

TSharedPtr<IAnalyticsProvider> UFortAnalytics::CreateAnalyticsProvider(const FString& UserID, const FString& GameSessionID, const FString& Platform, EAnalyticsProviderType ProviderType, const FString& GameState)
{
    static auto CreateAnalyticsProvider = Anchor::Function<TSharedPtr<IAnalyticsProvider> (*)(const FString&, const FString&, const FString&, EAnalyticsProviderType, const FString&)>(TEXT("UFortAnalytics::CreateAnalyticsProvider"))
        .String(TEXT("%s|[EMPTY]"))
        .Start()
        .Or()
        .String(TEXT("%s|%s|%s|%s|"), "lea *, [0x*]; mov r9, *; mov [rsp+0x20], *; mov r8, *; lea rcx, [rbp*]; call 0x*")
        .Start()
        .Or()
        .String("AnalyticsET")
        .Start()
        .Get();

    return CreateAnalyticsProvider(UserID, GameSessionID, Platform, ProviderType, GameState);
}

void UFortAnalytics::FireEvent_BuildingAction(AFortPlayerController* PlayerController, const TCHAR* ActionType, ABuildingActor* BuildingActor, int32 ResourcesUsed)
{
    static auto FireEvent_BuildingAction = Anchor::Function<void (*)(AFortPlayerController*, const TCHAR*, ABuildingActor*, int32)>(TEXT("UFortAnalytics::FireEvent_BuildingAction"))
        .String(TEXT("Building.BuildingAction"))
        .Start()
        .Get();

    FireEvent_BuildingAction(PlayerController, ActionType, BuildingActor, ResourcesUsed);
}
