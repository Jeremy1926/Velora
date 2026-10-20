#include "pch.h"
#include "AI/Navigation/FortNavMesh.h"
#include "Scanner.h"

void AFortNavMesh::SetUseGenerationBudgets(bool bUseGenerationBudgets)
{
    static auto SetUseGenerationBudgets = Anchor::Function<void (*)(AFortNavMesh*, bool)>(TEXT("AFortNavMesh::SetUseGenerationBudgets"))
        .String(TEXT("AFortGameSessionDedicatedAthena: Removing navigation locks"))
        .Find("xor edx, edx; mov rcx, *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    SetUseGenerationBudgets(this, bUseGenerationBudgets);
}
