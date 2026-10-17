#pragma once

#include "UObject/Object.h"

class ULocalPlayer;

class UGameInstance : public UObject
{
public:
    bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer);

    ULocalPlayer* GetFirstGamePlayer() const;
};
