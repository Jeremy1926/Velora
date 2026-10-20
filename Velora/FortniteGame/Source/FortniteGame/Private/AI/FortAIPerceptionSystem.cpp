#include "pch.h"
#include "AI/FortAIPerceptionSystem.h"
#include "Scanner.h"

void UFortAIPerceptionSystem::ForceForgetEnemy(AActor* Enemy)
{
    static auto ForceForgetEnemy = Scanner::Function<void (*)(AActor*)>([]
    {
        auto PlayerSpawned = Scanner::String("PlayerSpawned").Start();
        auto ForceForgetEnemy = PlayerSpawned.Find("lea rdx, [rbp-0x*]; mov rcx, *; call [rax+0x*]; lea rcx, *; call 0x*").Call(2);

        if (!ForceForgetEnemy.Get())
            ForceForgetEnemy = PlayerSpawned.Find("lea rdx, [rbp-0x*]; mov rcx, *; call [rax+0x*]").Find("mov rcx, r*; call 0x*").Call();

        return ForceForgetEnemy.Get(TEXT("UFortAIPerceptionSystem::ForceForgetEnemy"));
    }());

    ForceForgetEnemy(Enemy);
}
