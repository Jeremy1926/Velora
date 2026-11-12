#include "pch.h"
#include "ContentStreaming.h"
#include "Containers/Array.h"
#include "Scanner.h"

FStreamingManagerCollection& IStreamingManager::Get()
{
    static auto Get = Anchor::Function<FStreamingManagerCollection& (*)()>(TEXT("IStreamingManager::Get"))
        .String(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)"))
        .Start()
        .Find("call 0x*; xor r8d, r8d; movaps xmm1, xmm*; mov rcx, rax")
        .Target()
        .Or()
        .String(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)"))
        .Start()
        .Find("call 0x*; xor r8d, r8d; movaps xmm1, xmm*; mov rcx, [rax]")
        .Target()
        .Or()
        .String(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)"))
        .Start()
        .Find("mov *, [0x*]; test *, *; jz *; mov rax, [*]; mov rcx, *; movaps xmm1, xmm*; xor r8d, r8d; call [rax+0x8]")
        .Target()
        .Reference("mov *, [0x*]; test *, *; jz *; mov rax, *; add rsp, 0x*")
        .Start()
        .Or()
        .String(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)"))
        .Start()
        .Find("mov rcx, [0x*]; *; *; *; test rcx, rcx; jnz 0x*; lea edx, [rcx+0x10]")
        .Target()
        .Reference("mov rax, [0x*]; test rax, rax; jnz 0x*; lea edx, [rax+0x10]; lea ecx, [rax+0x*]; call 0x*; test rax, rax; jz 0x*; mov rcx, rax; call 0x*; mov [0x*], rax; add rsp, 0x28")
        .Start()
        .Get();

    return Get();
}

void IStreamingManager::RemoveStreamingViews(ERemoveStreamingViews RemovalType)
{
    static auto RemoveStreamingViews = Anchor::Function<void (*)(IStreamingManager*, ERemoveStreamingViews)>(TEXT("IStreamingManager::RemoveStreamingViews"))
        .Pattern("40 53 48 83 EC 20 83 3D ? ? ? ? 00 8B DA C7 05 ? ? ? ? 00 00 00 00")
        .Or()
        .Pattern("40 53 48 83 EC 20 83 25 ? ? ? ? 00 8B DA 83 3D ? ? ? ? 00")
        .Or()
        .Pattern("40 53 48 83 EC 20 8B DA 48 8D 0D ? ? ? ? 33 D2 E8 ? ? ? ? 83 25 ? ? ? ? 00 83 3D ? ? ? ? 00")
        .Or()
        .Pattern("40 53 48 83 EC 30 83 25 ? ? ? ? 00 8B DA 83 3D ? ? ? ? 00")
        .Or()
        .Pattern("56 48 83 EC 30 89 D6 C7 05 ? ? ? ? 00 00 00 00 83 3D ? ? ? ? 00 74 ? 48 8D 05 ? ? ? ? 48 89 44 24 28")
        .Optional()
        .Get();

    if (RemoveStreamingViews)
    {
        RemoveStreamingViews(this, RemovalType);
        return;
    }

    static auto PendingViewInfos = Anchor::Data<TArray<uint8>*>(TEXT("IStreamingManager::PendingViewInfos"))
        .String(TEXT("Loading Map %s"))
        .Start()
        .Find("cmp dword ptr [0x*], 0x0; mov [0x*], *; jz 0x*; xor edx, edx; lea rcx, [0x*]; call 0x*; cmp dword ptr [0x*], 0x0; mov [0x*], *; jz 0x*; call 0x*; cmp dword ptr [0x*], 0x0; mov [0x*], *; jz 0x*; xor edx, edx; lea rcx, [0x*]; call 0x*")
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    static auto SecondaryLocations = Anchor::Data<TArray<uint8>*>(TEXT("IStreamingManager::SecondaryLocations"))
        .String(TEXT("Loading Map %s"))
        .Start()
        .Find("cmp dword ptr [0x*], 0x0; mov [0x*], *; jz 0x*; xor edx, edx; lea rcx, [0x*]; call 0x*; cmp dword ptr [0x*], 0x0; mov [0x*], *; jz 0x*; call 0x*; cmp dword ptr [0x*], 0x0; mov [0x*], *; jz 0x*; xor edx, edx; lea rcx, [0x*]; call 0x*")
        .Find("call 0x*", 2)
        .Target()
        .Find("lea rcx, [0x*]")
        .Target()
        .Get();

    static auto LastingViewInfos = Anchor::Data<TArray<uint8>*>(TEXT("IStreamingManager::LastingViewInfos"))
        .String(TEXT("Loading Map %s"))
        .Start()
        .Find("cmp dword ptr [0x*], 0x0; mov [0x*], *; jz 0x*; xor edx, edx; lea rcx, [0x*]; call 0x*; cmp dword ptr [0x*], 0x0; mov [0x*], *; jz 0x*; call 0x*; cmp dword ptr [0x*], 0x0; mov [0x*], *; jz 0x*; xor edx, edx; lea rcx, [0x*]; call 0x*")
        .Find("lea rcx, [0x*]", 2)
        .Target()
        .Get();

    PendingViewInfos->Empty();
    SecondaryLocations->Empty();

    if (RemovalType == RemoveStreamingViews_All)
        LastingViewInfos->Empty();
}
