#pragma once

#include "GameFramework/Controller.h"

class UCheatManager;

class APlayerController : public AController
{
public:
    void SendClientAdjustment();

    void ServerAcknowledgePossession_Implementation(APawn* P);

    void ServerRestartPlayer_Implementation();

    UCheatManager* GetCheatManager() const;

    void SetCheatManager(UCheatManager* InCheatManager);

    UClass* GetCheatClass() const;

    void AddCheats(bool bForce = false);
};
