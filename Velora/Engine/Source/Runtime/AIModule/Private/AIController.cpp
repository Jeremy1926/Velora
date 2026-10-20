#include "pch.h"
#include "AIController.h"
#include "Scanner.h"

bool AAIController::RunBehaviorTree(UBehaviorTree* BTAsset)
{
    static auto RunBehaviorTree = Anchor::VirtualFunction<bool (*)(AAIController*, UBehaviorTree*)>(TEXT("AAIController::RunBehaviorTree"))
        .Exec(TEXT("/Script/AIModule.AIController:RunBehaviorTree"))
        .Find("call [rax+0x*]")
        .Get();

    return RunBehaviorTree(this, BTAsset);
}
