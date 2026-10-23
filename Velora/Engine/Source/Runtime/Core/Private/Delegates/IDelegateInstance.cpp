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
        .Get();

    if (!NextID)
        return 0;

    auto Result = (uint64)_InterlockedIncrement64(NextID);

    if (!Result)
        Result = (uint64)_InterlockedIncrement64(NextID);

    return Result;
}

bool IDelegateInstance::IsDestructorFirst()
{
    static auto bDestructorFirst = []
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

            for (int64 Index = 0; Index + 4 < Count; Index++)
            {
                if (VTable[Index + 1] != VTable[Index] || VTable[Index + 2] != VTable[Index] || !Anchor::IsCode(VTable[Index]) || !Anchor::IsCode(VTable[Index + 3]) || !Anchor::IsCode(VTable[Index + 4]))
                    continue;

                if (!Anchor::At(VTable[Index]).Is("xor eax, eax; ret") || !Anchor::At(VTable[Index + 3]).Is("xor al, al; ret"))
                    continue;

                if (Anchor::At(VTable[Index + 4]).Is("sub rsp, 0x28; mov rax, [rcx]; call [rax+0x28]"))
                    return false;

                if (Anchor::At(VTable[Index + 4]).Is("sub rsp, 0x28; mov rax, [rcx]; call [rax+0x30]") || Anchor::At(VTable[Index + 4]).Is("sub rsp, 0x28; mov rax, [rcx]; mov rdx, [rax+0x30]"))
                    return true;
            }
        }

        return false;
    }();

    return bDestructorFirst;
}
