#include "pch.h"
#include "ReplicationGraphTypes.h"
#include "Scanner.h"

void FGlobalActorReplicationInfoMap::ResetActorMap()
{
    static auto DestructItem = Anchor::Function<void (*)(void*)>(TEXT("TPair<FActorRepListType, TUniquePtr<FGlobalActorReplicationInfo>>::~TPair"))
        .String(TEXT("UReplicationGraph::FlushNetDormancy %s. WantsToBeDormant is changing*"))
        .Start()
        .SlotIn(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"))
        .SlotShift(TEXT("/Script/ReplicationGraph.Default__ReplicationGraph"), -0x10)
        .Find("lea rcx, [*+0x*]; lea rdx, [*]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("pop rdi; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Find("lea rcx, [rax+r8*8]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Get();

    if (!ActorMap.Num())
        return;

    for (auto& Pair : ActorMap)
        DestructItem((void*)&Pair);

    ActorMap.Reset();
}
