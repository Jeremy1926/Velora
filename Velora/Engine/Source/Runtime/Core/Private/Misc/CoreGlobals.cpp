#include "pch.h"
#include "CoreGlobals.h"
#include "Scanner.h"

bool& GIsClient = *Anchor::Data<bool*>(TEXT("GIsClient"))
    .String(TEXT("Cannot run editor commandlet %s with game executable."))
    .Last("mov [0x*], ?l")
    .Last("mov [0x*], ?l")
    .Target()
    .Get();

bool& GIsServer = *Anchor::Data<bool*>(TEXT("GIsServer"))
    .String(TEXT("Cannot run editor commandlet %s with game executable."))
    .Last("mov [0x*], ?l")
    .Target()
    .Get();

bool& GIsGameThreadIdInitialized = *Anchor::Data<bool*>(TEXT("GIsGameThreadIdInitialized"))
    .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
    .Start()
    .Find("cmp byte ptr [0x*], 0x0")
    .Target()
    .Get();

uint32& GGameThreadId = *Anchor::Data<uint32*>(TEXT("GGameThreadId"))
    .String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms"))
    .Start()
    .Find("cmp byte ptr [0x*], 0x0")
    .Find("cmp *, [0x*]")
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
        .Get();

    return GetGlobalLogSingleton();
}
