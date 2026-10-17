#pragma once

#include "UObject/Object.h"
#include "Containers/Array.h"

class ULocalPlayer;
struct FWorldContext;

class UGameInstance : public UObject
{
public:
    TArray<ULocalPlayer*>& GetLocalPlayers();

    ULocalPlayer* GetFirstGamePlayer() const;

    FWorldContext* GetWorldContext() const;
};
