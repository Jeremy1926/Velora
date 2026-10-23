#pragma once

#include "GameFramework/Actor.h"

class AController;
class APlayerState;

class APawn : public AActor
{
public:
    static UClass* StaticClass();

    AController* GetController() const;

    APlayerState* GetPlayerState() const;
};
