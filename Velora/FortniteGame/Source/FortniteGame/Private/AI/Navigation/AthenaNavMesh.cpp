#include "pch.h"
#include "AI/Navigation/AthenaNavMesh.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

void AAthenaNavMesh::RebuildAll()
{
    DETOUR(AAthenaNavMesh, RebuildAll, Anchor::Hook()
        .String(TEXT("   Building NavData:  %s."))
        .Find("ret")
        .Last("call [rax+0x*]")
        .IsVirtual(TEXT("/Script/FortniteGame.Default__AthenaNavMesh"), "ret*")
        .Or()
        .String(TEXT("   RebuildAll building NavData:  %s."))
        .Find("ret")
        .Last("call [rax+0x*]")
        .IsVirtual(TEXT("/Script/FortniteGame.Default__AthenaNavMesh"), "ret*")
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__AthenaNavMesh")));

    static auto RebuildAll = Anchor::VirtualFunction<void (*)(AAthenaNavMesh*)>(TEXT("ANavigationData::RebuildAll"))
        .String(TEXT("   Building NavData:  %s."))
        .Find("ret")
        .Last("call [rax+0x*]")
        .Or()
        .String(TEXT("   RebuildAll building NavData:  %s."))
        .Find("ret")
        .Last("call [rax+0x*]")
        .Get(TEXT("/Script/FortniteGame.Default__FortNavMesh"));

    RebuildAll(this);
}
