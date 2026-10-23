#pragma once

#include "Engine/GameInstance.h"
#include "Templates/SharedPointer.h"

class IAnalyticsProvider;
class UFortPlaylistManager;
class UUpdateManager;

class UFortGameInstance : public UGameInstance
{
public:
    UFortPlaylistManager* GetPlaylistManager() const;

    UUpdateManager* GetUpdateManager() const;

    TSharedPtr<IAnalyticsProvider>* GetServerAnalyticsProvider();

    TSharedPtr<IAnalyticsProvider>* GetServerEsportsAnalyticsProvider();
};
