#pragma once

#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"

class ABuildingActor;
class AFortPlayerController;
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

    static void FireEvent_BuildingAction(AFortPlayerController* PlayerController, const TCHAR* ActionType, ABuildingActor* BuildingActor, int32 ResourcesUsed);
};
