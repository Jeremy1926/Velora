#pragma once

#include "UObject/Object.h"

class ULocalPlayer;
struct FWorldContext;

class UGameInstance : public UObject
{
public:
    bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer);

    ULocalPlayer* GetFirstGamePlayer() const;

    FWorldContext* GetWorldContext() const;
};
