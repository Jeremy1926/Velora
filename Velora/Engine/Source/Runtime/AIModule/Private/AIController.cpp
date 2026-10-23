#include "pch.h"
#include "AIController.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"

UClass* AAIController::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/AIModule.AIController"));

    return Class;
}

bool AAIController::RunBehaviorTree(UBehaviorTree* BTAsset)
{
    static auto RunBehaviorTree = Anchor::VirtualFunction<bool (*)(AAIController*, UBehaviorTree*)>(TEXT("AAIController::RunBehaviorTree"))
        .Exec(TEXT("/Script/AIModule.AIController:RunBehaviorTree"))
        .Find("call [rax+0x*]")
        .Get();

    return RunBehaviorTree(this, BTAsset);
}

UAIPerceptionComponent* AAIController::GetPerceptionComponent() const
{
    static auto PerceptionComponent = StaticClass()->FindPropertyByName(FName(TEXT("PerceptionComponent")));

    return *PerceptionComponent->ContainerPtrToValuePtr<UAIPerceptionComponent*>(this);
}
