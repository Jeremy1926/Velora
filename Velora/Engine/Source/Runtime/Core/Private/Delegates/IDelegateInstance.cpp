#include "pch.h"
#include "Delegates/IDelegateInstance.h"
#include "Scanner.h"

uint64 FDelegateHandle::GenerateNewID()
{
    static auto NextID = Anchor::Data<volatile int64*>(TEXT("FDelegateHandle::GenerateNewID::NextID"))
        .Exec(TEXT("/Script/FortniteGame.AthenaAISpawner:RequestSpawn"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Optional()
        .Find("call 0x*")
        .Target()
        .Find("mov dword ptr [rsp+0x*], 0x3; test rax, rax; jz 0x*; mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("lock xadd [0x*], *")
        .Target()
        .Get();

    if (!NextID)
        return 0;

    auto Result = (uint64)_InterlockedIncrement64(NextID);

    if (!Result)
        Result = (uint64)_InterlockedIncrement64(NextID);

    return Result;
}
