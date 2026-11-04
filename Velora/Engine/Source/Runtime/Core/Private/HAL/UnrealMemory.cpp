#include "pch.h"
#include "HAL/UnrealMemory.h"
#include "Containers/Set.h"
#include "Scanner.h"

void* FMemory::Realloc(void* Original, SIZE_T Count, uint32 Alignment)
{
    static auto Realloc = Anchor::Function<void* (*)(void*, SIZE_T, uint32)>(TEXT("FMemory::Realloc"))
        .Pattern("4C 8B D1 48 8B 0D ? ? ? ? 48 85 C9 75 08 49 8B CA E9 ? ? ? ? 48 8B 01 45 8B C8 4C 8B C2 49 8B D2 48 FF 60 18")
        .Or()
        .Pattern("48 8B F1 41 8B D8 48 8B 0D ? ? ? ? 48 8B FA 48 85 C9 75 0C E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 44 8B CB 4C 8B C7 48 8B D6 48 8B 5C 24 30 48 8B 74 24 38 48 83 C4 20 5F 48 FF 60 18")
        .Start()
        .Or()
        .Pattern("48 8B F1 41 8B D8 48 8B 0D ? ? ? ? 48 8B FA 48 85 C9 75 0C E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 44 8B CB 4C 8B C7 48 8B D6 48 8B 5C 24 30 48 8B 74 24 38 48 83 C4 20 5F 48 FF 60 20")
        .Start()
        .Or()
        .Pattern("48 8B F1 41 8B D8 48 8B 0D ? ? ? ? 48 8B FA 48 85 C9 75 0C E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B 01 44 8B CB 4C 8B C7 48 8B D6 48 8B 5C 24 30 48 8B 74 24 38 48 83 C4 20 5F 48 FF 60 ?")
        .Start()
        .Or()
        .Pattern("56 57 53 48 83 EC 20 44 89 C6 48 89 D7 48 89 CB 48 8B 0D ? ? ? ? 48 85 C9 74 ? 48 8B 01 48 8B 40 38 48 89 DA 49 89 F8 41 89 F1 48 83 C4 20 5B 5F 5E 48 FF E0")
        .Or()
        .Pattern("53 ? 83 ? ? ? 8B D9 ? 8B 09 ? 85 C9 75 ? ? 85 C0 74 ? ? 85 C0 0F 88 ? ? ? ? ? 8D 41 FF ? 3D FE FF FF 7F 0F 87 ? ? ? ? ? 63 D0 ? 33 C0 ? 0F AF D1 E8")
        .Find("call 0x*")
        .Target()
        .Or()
        .Pattern("53 ? 83 ? ? ? 8B D9 ? 8B 09 ? 85 C9 75 ? ? 85 C0 74 ? ? 85 C0 78 ? ? 8D 41 FF ? 3D FE FF FF 7F 77 ? ? 63 D0 ? 33 C0 ? 0F AF D1 E8")
        .Find("call 0x*")
        .Target()
        .Get();

    return Realloc(Original, Count, Alignment);
}

void FMemory::Free(void* Original)
{
    static auto Free = Anchor::Function<void (*)(void*)>(TEXT("FMemory::Free"))
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"))
        .Start()
        .Find("test rcx, rcx; jz *; call 0x*")
        .Call()
        .Or()
        .String(TEXT("Illegal call to StaticFindObject() while collecting garbage!"))
        .Start()
        .Find("test rcx, rcx; jnz *; xor eax, eax")
        .Find("jnz 0x*")
        .Target()
        .Is("call 0x*")
        .Call()
        .Or()
        .Pattern("45 33 F6 4C 8B E1 45 0F B6 E9 49 8B F8 41 8B C6 41 8B CE 48 83 FA FF 0F 84")
        .Start()
        .Find("test rcx, rcx; jz *; call 0x*")
        .Call()
        .Or()
        .String(TEXT("GlobalUMGSequenceTickManager"))
        .Find("call 0x*")
        .Target()
        .Find("test rcx, rcx; jnz *; xor eax, eax")
        .Find("jnz 0x*")
        .Target()
        .Is("call 0x*")
        .Call()
        .Or()
        .String(TEXT("GlobalUMGSequenceTickManager"), "lea r8, [0x*]; mov rdx, rbx; mov rcx, rax; add rsp, 0x*; pop rbx; jmp 0x*")
        .Find("jmp 0x*")
        .Target()
        .Find("test rcx, rcx; jz *; call 0x*")
        .Call()
        .Or()
        .String(TEXT("GlobalUMGSequenceTickManager"))
        .Find("call 0x*")
        .Target()
        .Find("call 0x*; xor eax, eax")
        .Target()
        .Find("jmp 0x*")
        .Target()
        .Find("test rcx, rcx; jz *; call 0x*")
        .Call()
        .Or()
        .String(TEXT("GlobalUMGSequenceTickManager"))
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [rbp-0x*]; mov ebx, [rax]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("jnz 0x*")
        .Target()
        .Is("jmp 0x*")
        .Target()
        .Is("test rcx, rcx; jz *; push rbx")
        .Or()
        .Pattern("40 53 48 83 EC 20 48 8B D9 48 8B 0D ? ? ? ? 48 85 C9 75 ? E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 85 DB 74 ? 48 8B 01 48 8B D3 FF 50 ? 48 83 C4 20 5B C3")
        .Or()
        .String(TEXT("GlobalUMGSequenceTickManager"))
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [rbp-0x*]; mov *, [rax]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("jnz 0x*")
        .Target()
        .Is("jmp 0x*")
        .Target()
        .Is("test rcx, rcx; jz *; push rdi")
        .Or()
        .Pattern("48 85 C9 0F 84 ? ? ? ? 57 48 83 EC 70 4C 89 74 24 40 48 8B F9 4C 8B 35 ? ? ? ? 4D 85 F6 0F 84 ? ? ? ? 49 8B 06 4C 8B 40 48")
        .Or()
        .String(TEXT("GlobalUMGSequenceTickManager"))
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [rbp-0x*]; mov *, [rax]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("jnz 0x*")
        .Target()
        .Is("jmp 0x*")
        .Target()
        .Is("test rcx, rcx; jz *; mov r11, rsp; push rdi")
        .Or()
        .String(TEXT("GlobalUMGSequenceTickManager"))
        .Find("call 0x*")
        .Target()
        .Find("lea rcx, [rbp-0x*]; mov *, [rax]; call 0x*")
        .Find("call 0x*")
        .Target()
        .Find("jnz 0x*")
        .Target()
        .Is("jmp 0x*")
        .Target()
        .Is("test rcx, rcx; jz *; push rbx")
        .Or()
        .Pattern("48 85 C9 0F 84 ? ? ? ? 57 48 83 EC 70 48 89 74 24 58 48 8B F9 48 8B 35 ? ? ? ? 48 85 F6 0F 84 ? ? ? ? 48 8B 06 4C 8B 40 48")
        .Or()
        .Pattern("48 8B F9 48 8B 35 ? ? ? ? 48 85 F6 0F 84 ? ? ? ? 48 8B 06 4C 8B 40 48")
        .Start()
        .Or()
        .Exec(TEXT("/Script/Engine.KismetStringLibrary:Conv_NameToString"))
        .Find("sbb rax, 0xffffffffffffffff; mov [*+0x20], rax")
        .Find("mov rcx, [r*]; test rcx, rcx; jz *; call 0x*")
        .Find("call 0x*")
        .Target()
        .Or()
        .String(TEXT("Failed to find object '{ClassName} {OuterName}.{ObjectName}'"))
        .Start()
        .Find("test rcx, rcx; jz *; call 0x*")
        .Call()
        .Or()
        .Pattern("85 C9 0F 84 ? ? ? ? 55 56 ? 8D 6C 24 A8 ? 81 ? ? ? ? ? ? 8B 05 ? ? ? ? ? 33 C4 ? 89 45 30 ? 89 B4 24 48 01 00 00")
        .Start()
        .Or()
        .Pattern("85 C9 0F 84 ? ? ? ? 56 ? 81 ? ? ? ? ? ? 8B 05 ? ? ? ? ? 33 C4 ? 89 84 24 30 01 00 00 ? 89 B4 24 48 01 00 00")
        .Start()
        .Get();

    Free(Original);
}

SIZE_T FMemory::QuantizeSize(SIZE_T Count, uint32 Alignment)
{
    static auto QuantizeSize = Anchor::Optional<SIZE_T (*)(SIZE_T, uint32)>(TEXT("FMemory::QuantizeSize"))
        .Pattern("4C 8B 0D ? ? ? ? 4D 85 C9 75 04 48 8B C1 C3 49 8B 01 44 8B C2 48 8B D1 49 8B C9 48 FF 60 28")
        .Or()
        .Pattern("4C 8B 0D ? ? ? ? 4D 85 C9 75 04 48 8B C1 C3 49 8B 01 44 8B C2 48 8B D1 49 8B C9 48 FF 60 38")
        .Get();

    return QuantizeSize ? QuantizeSize(Count, Alignment) : Count;
}

bool FCompactSetBase::IsEnabled()
{
    static auto bCompact = Anchor::Exists()
        .Exec(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:SetNamedParam"))
        .Find("setnz *; add [*+0x20], *")
        .Call()
        .Call()
        .Find("movsxd *, dword ptr [rcx+0xc]; xor *, *; cmp *, 0x10000")
        .Get();

    return bCompact;
}

int32 FCompactSetBase::AllocatorCalculateSlackGrow(int32 NewMaxElements, const FCompactSetLayout& Layout) const
{
    static auto AllocatorCalculateSlackGrow = Anchor::Function<int32 (*)(const FCompactSetBase*, int32, const FCompactSetLayout&)>(TEXT("TCompactSetBase::AllocatorCalculateSlackGrow"))
        .Exec(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:SetNamedParam"))
        .Find("setnz *; add [*+0x20], *")
        .Call()
        .Find("lea edx, [*+0x1]; mov rcx, *; call 0x*; mov rcx, *; mov edx, eax; call 0x*")
        .Call()
        .Get();

    return AllocatorCalculateSlackGrow(this, NewMaxElements, Layout);
}

bool FCompactSetBase::ResizeAllocationPreserveData(int32 NewMaxElements, const FCompactSetLayout& Layout, bool bPreserve)
{
    static auto ResizeAllocationPreserveData = Anchor::Function<bool (*)(FCompactSetBase*, int32, const FCompactSetLayout&, bool)>(TEXT("TCompactSetBase::ResizeAllocationPreserveData"))
        .Exec(TEXT("/Script/AIModule.EnvQueryInstanceBlueprintWrapper:SetNamedParam"))
        .Find("setnz *; add [*+0x20], *")
        .Call()
        .Find("lea edx, [*+0x1]; mov rcx, *; call 0x*; mov rcx, *; mov edx, eax; call 0x*")
        .Call(2)
        .Find("mov r9b, 0x1; call 0x*")
        .Call()
        .Get();

    return ResizeAllocationPreserveData(this, NewMaxElements, Layout, bPreserve);
}
