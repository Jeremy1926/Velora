#pragma once

#include "GameFramework/Controller.h"
#include "Containers/UnrealString.h"

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

    FString ConsoleCommand(const FString& Command, bool bWriteToLog = true);

    void ClientMessage(const FString& S, FName Type = NAME_None, float MsgLifeTime = 0.f);
};
