#pragma once

#include "GameFramework/Controller.h"

class UBehaviorTree;

class AAIController : public AController
{
public:
    bool RunBehaviorTree(UBehaviorTree* BTAsset);
};
