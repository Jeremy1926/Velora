#include "pch.h"
#include "AI/FortAIPerceptionSystem.h"
#include "Scanner.h"

void UFortAIPerceptionSystem::ForceForgetEnemy(AActor* Enemy)
{
    static auto ForceForgetEnemy = Scanner::Function<void (*)(AActor*)>(Scanner::String("PlayerSpawned").Start().Find("lea rcx, *; call 0x*; mov rcx, *; call 0x*; mov rcx, *; call 0x*").Call(2).Get(TEXT("UFortAIPerceptionSystem::ForceForgetEnemy")));

    ForceForgetEnemy(Enemy);
}
