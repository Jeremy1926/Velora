#include "pch.h"
#include "CoreGlobals.h"
#include "Scanner.h"

bool& GIsClient = *Anchor::Data<bool*>(TEXT("GIsClient"))
    .String(TEXT("Cannot run editor commandlet %s with game executable."))
    .Last("mov [0x*], ?l; test byte ptr [*], 0x4; mov [0x*], ?l")
    .Find("mov [0x*], ?l")
    .Target()
    .Or()
    .String(TEXT("Cannot run editor commandlet %s with game executable."))
    .Last("mov [0x*], ?l")
    .Last("mov [0x*], ?l")
    .Target()
    .Get();

bool& GIsServer = *Anchor::Data<bool*>(TEXT("GIsServer"))
    .String(TEXT("Cannot run editor commandlet %s with game executable."))
    .Last("mov [0x*], ?l; test byte ptr [*], 0x4; mov [0x*], ?l")
    .Target()
    .Or()
    .String(TEXT("Cannot run editor commandlet %s with game executable."))
    .Last("mov [0x*], ?l")
    .Target()
    .Get();

bool& GIsGameThreadIdInitialized = *Anchor::Data<bool*>(TEXT("GIsGameThreadIdInitialized"))
    .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
    .Start()
    .Find("cmp byte ptr [0x*], 0x0")
    .Target()
    .Or()
    .String(TEXT("UpdateMapTexture() failed.  FogMask or FogMask->Resource is null."))
    .Start()
    .Find("cmp byte ptr [0x*], 0x0; jz *; call [0x*]; cmp eax, [0x*]")
    .Target()
    .Get();

uint32& GGameThreadId = *Anchor::Data<uint32*>(TEXT("GGameThreadId"))
    .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
    .Start()
    .Find("cmp byte ptr [0x*], 0x0")
    .Find("cmp *, [0x*]")
    .Target()
    .Or()
    .String(TEXT("UpdateMapTexture() failed.  FogMask or FogMask->Resource is null."))
    .Start()
    .Find("cmp byte ptr [0x*], 0x0; jz *; call [0x*]; cmp eax, [0x*]")
    .Find("cmp eax, [0x*]")
    .Target()
    .Get();

uint32* GSlateLoadingThreadId = Anchor::Optional<uint32*>(TEXT("GSlateLoadingThreadId"))
    .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
    .Start()
    .Find("cmp byte ptr [0x*], 0x0")
    .Find("cmp *, [0x*]; jz *; cmp *, [0x*]")
    .Find("cmp *, [0x*]")
    .Target()
    .Get();

FOutputDeviceRedirector* GetGlobalLogSingleton()
{
    static auto GetGlobalLogSingleton = Anchor::Function<FOutputDeviceRedirector* (*)()>(TEXT("GetGlobalLogSingleton"))
        .String(TEXT("NOCONSOLE"))
        .Call(2)
        .Or()
        .String(TEXT("STAT_FEngineLoop_FlushThreadedLogs"))
        .Find("call 0x*; mov rcx, rax; mov rdx, [rax]; call [rdx+0x48]")
        .Call()
        .Get();

    return GetGlobalLogSingleton();
}
