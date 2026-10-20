#include "pch.h"
#include "AI/Navigation/AthenaNavMesh.h"
#include "UObject/UObjectGlobals.h"
#include "Scanner.h"
#include "Detour.h"

UClass* AAthenaNavMesh::StaticClass()
{
    static auto Class = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.AthenaNavMesh"));

    return Class;
}

void AAthenaNavMesh::RebuildAll()
{
    DETOUR(AAthenaNavMesh, RebuildAll, Anchor::Hook()
        .String(TEXT("   Building NavData:  %s."))
        .Find("ret")
        .Last("call [rax+0x*]")
        .IsVirtual(TEXT("/Script/FortniteGame.Default__AthenaNavMesh"), "ret*")
        .Optional()
        .InVTable(TEXT("/Script/FortniteGame.Default__AthenaNavMesh")));

    static auto RebuildAll = Anchor::VirtualFunction<void (*)(AAthenaNavMesh*)>(TEXT("ANavigationData::RebuildAll"))
        .String(TEXT("   Building NavData:  %s."))
        .Find("ret")
        .Last("call [rax+0x*]")
        .Get(TEXT("/Script/FortniteGame.Default__FortNavMesh"));

    RebuildAll(this);
}
