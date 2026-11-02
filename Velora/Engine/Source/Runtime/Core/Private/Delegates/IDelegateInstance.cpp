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
        .Or()
        .Exec(TEXT("/Script/FortniteGame.AthenaAISpawner:RequestSpawn"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Find("mov dword ptr [rsp+0x*], 0x3; test rax, rax; jz 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("lock xadd [0x*], *")
        .Target()
        .Or()
        .Exec(TEXT("/Script/FortniteGame.AthenaAISpawner:RequestSpawn"))
        .Find("setnz *")
        .Find("add [*+0x20], *")
        .Find("call 0x*")
        .Target()
        .Find("movdqa [rsp+0x*], xmm0; mov rcx, rax; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("call 0x*")
        .Target()
        .Find("lock xadd [0x*], *")
        .Target()
        .Or()
        .Pattern("41 8B C0 F0 48 0F C1 05 ? ? ? ? 49 03 C0 75 0F 41 8B C0 F0 48 0F C1 05 ? ? ? ? 48 FF C0")
        .Find("lock xadd [0x*], *")
        .Target()
        .Or()
        .Pattern("41 8B C0 F0 48 0F C1 05 ? ? ? ? 49 03 C0 49 89 43 08 75 13 41 8B C0 F0 48 0F C1 05 ? ? ? ? 48 FF C0")
        .Find("lock xadd [0x*], *")
        .Target()
        .Or()
        .Pattern("BA 01 00 00 00 48 89 01 8B C2 F0 48 0F C1 05 ? ? ? ? 48 03 C2 0F 84")
        .Find("lock xadd [0x*], *")
        .Target()
        .Or()
        .Pattern("41 B9 01 00 00 00 4C 8B C1 41 8B D1 F0 48 0F C1 15 ? ? ? ? 48 8B 01 48 FF C2 48 89 10")
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

int32 IDelegateInstance::GetLayout()
{
    static auto Layout = []
    {
        auto Image = Anchor::Base();
        auto Headers = (IMAGE_NT_HEADERS*)(Image + ((IMAGE_DOS_HEADER*)Image)->e_lfanew);
        auto Section = IMAGE_FIRST_SECTION(Headers);

        for (int32 SectionIndex = 0; SectionIndex < Headers->FileHeader.NumberOfSections; SectionIndex++, Section++)
        {
            if (strncmp((const char*)Section->Name, ".rdata", sizeof(Section->Name)))
                continue;

            auto VTable = (const uint8* const*)(Image + Section->VirtualAddress);
            auto Count = (int64)(Section->Misc.VirtualSize / sizeof(void*));

            for (int64 Index = 0; Index + 9 < Count; Index++)
            {
                if (VTable[Index + 1] != VTable[Index] || VTable[Index + 2] != VTable[Index] || !Anchor::IsCode(VTable[Index]) || !Anchor::IsCode(VTable[Index + 3]) || !Anchor::IsCode(VTable[Index + 4]))
                    continue;

                if (!Anchor::At(VTable[Index]).Is("xor eax, eax; ret") || !Anchor::At(VTable[Index + 3]).Is("xor al, al; ret") && !Anchor::At(VTable[Index + 4]).Is("sub rsp, 0x28; mov rax, [rcx]; mov rax, [rax+0x30]; lea rdx, [0x*]; cmp rax, rdx"))
                    continue;

                if (Anchor::At(VTable[Index + 4]).Is("sub rsp, 0x28; mov rax, [rcx]; call [rax+0x28]"))
                    return 0;

                if (Anchor::At(VTable[Index + 4]).Is("sub rsp, 0x28; mov rax, [rcx]; call [rax+0x30]") || Anchor::At(VTable[Index + 4]).Is("sub rsp, 0x28; mov rax, [rcx]; mov rdx, [rax+0x30]") || Anchor::At(VTable[Index + 4]).Is("sub rsp, 0x28; mov rax, [rcx]; mov rax, [rax+0x30]; lea rdx, [0x*]; cmp rax, rdx"))
                {
                    auto CreateCopyThreadSafe = Anchor::At(VTable[Index + 9]);

                    if (CreateCopyThreadSafe.Start().Get() != VTable[Index + 9])
                        return 1;

                    auto Lock = CreateCopyThreadSafe.Find("call [0x*]").Target();

                    if (!Lock)
                    {
                        auto ScopeLock = CreateCopyThreadSafe.Call();
                        Lock = ScopeLock.Get() && ScopeLock.Start().Get() == ScopeLock.Get() ? ScopeLock.Find("call [0x*]").Target() : nullptr;
                    }

                    return Lock && *(void* const*)Lock == (void*)EnterCriticalSection ? 2 : 1;
                }
            }
        }

        return 0;
    }();

    return Layout;
}
