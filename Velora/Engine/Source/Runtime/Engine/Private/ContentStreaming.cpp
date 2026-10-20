#include "pch.h"
#include "ContentStreaming.h"
#include "Scanner.h"

FStreamingManagerCollection& IStreamingManager::Get()
{
    static auto Get = Anchor::Function<FStreamingManagerCollection& (*)()>(TEXT("IStreamingManager::Get"))
        .String(TEXT("Slow GT frame detected (GT frame %u, delta time %f s)"))
        .Start()
        .Find("call 0x*; xor r8d, r8d; movaps xmm1, xmm*; mov rcx, rax")
        .Target()
        .Get();

    return Get();
}

void IStreamingManager::RemoveStreamingViews(ERemoveStreamingViews RemovalType)
{
    static auto RemoveStreamingViews = Anchor::Function<void (*)(IStreamingManager*, ERemoveStreamingViews)>(TEXT("IStreamingManager::RemoveStreamingViews"))
        .Pattern("40 53 48 83 EC 20 83 3D ? ? ? ? 00 8B DA C7 05 ? ? ? ? 00 00 00 00")
        .Get();

    RemoveStreamingViews(this, RemovalType);
}
