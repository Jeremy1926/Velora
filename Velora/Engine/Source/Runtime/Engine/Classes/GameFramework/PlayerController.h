#pragma once

#include "GameFramework/Controller.h"

class UCheatManager;

class APlayerController : public AController
{
public:
    void SendClientAdjustment();

    void ServerAcknowledgePossession_Implementation(APawn* P);

    void ServerRestartPlayer_Implementation();

    void Reset();

    void ClientGotoState(FName NewState);

    APawn* GetAcknowledgedPawn() const;

    APawn* GetSpectatorPawn() const;

    APawn* GetPawnOrSpectator() const;

    UCheatManager* GetCheatManager() const;

    void SetCheatManager(UCheatManager* InCheatManager);

    UClass* GetCheatClass() const;

    void AddCheats(bool bForce = false);
};
