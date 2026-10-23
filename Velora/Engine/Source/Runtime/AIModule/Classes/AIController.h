#pragma once

#include "GameFramework/Controller.h"

class UAIPerceptionComponent;
class UBehaviorTree;

class AAIController : public AController
{
public:
    static UClass* StaticClass();

    bool RunBehaviorTree(UBehaviorTree* BTAsset);

    UAIPerceptionComponent* GetPerceptionComponent() const;
};
