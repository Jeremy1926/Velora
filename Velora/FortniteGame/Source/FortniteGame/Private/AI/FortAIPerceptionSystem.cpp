#include "pch.h"
#include "AI/FortAIPerceptionSystem.h"
#include "Scanner.h"

void UFortAIPerceptionSystem::ForceForgetEnemy(AActor* Enemy)
{
    static auto ForceForgetEnemy = Anchor::Function<void (*)(AActor*)>(TEXT("UFortAIPerceptionSystem::ForceForgetEnemy"))
        .String("PlayerSpawned")
        .Start()
        .Find("lea rdx, [rbp-0x*]; mov rcx, *; call [rax+0x*]; lea rcx, *; call 0x*")
        .Call(2)
        .Or()
        .String("PlayerSpawned")
        .Start()
        .Find("lea rdx, [rbp-0x*]; mov rcx, *; call [rax+0x*]")
        .Find("mov rcx, r*; call 0x*")
        .Call()
        .Or()
        .String("PlayerSpawned")
        .Start()
        .Find("lea rdx, [rsp+0x*]; mov rcx, *; call [rax+0x*]; lea rcx, [rsp+0x*]; call 0x*; mov rcx, r*; call 0x*")
        .Call(2)
        .Get();

    ForceForgetEnemy(Enemy);
}
