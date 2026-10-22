#pragma once

#include "Engine/GameInstance.h"
#include "Templates/SharedPointer.h"

class IAnalyticsProvider;
class UFortPlaylistManager;

class UFortGameInstance : public UGameInstance
{
public:
    UFortPlaylistManager* GetPlaylistManager() const;

    TSharedPtr<IAnalyticsProvider>* GetServerAnalyticsProvider();

    TSharedPtr<IAnalyticsProvider>* GetServerEsportsAnalyticsProvider();
};
