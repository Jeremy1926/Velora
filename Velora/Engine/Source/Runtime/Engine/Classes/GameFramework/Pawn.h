#pragma once

#include "GameFramework/Actor.h"

class AController;

class APawn : public AActor
{
public:
    AController* GetController() const;
};
