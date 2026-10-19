#include "pch.h"
#include "CoreGlobals.h"
#include "Scanner.h"

bool& GIsClient = *(bool*)Scanner::String(TEXT("Cannot run editor commandlet %s with game executable.")).Last("mov [0x*], ?l").Last("mov [0x*], ?l").Target(TEXT("GIsClient"));
bool& GIsServer = *(bool*)Scanner::String(TEXT("Cannot run editor commandlet %s with game executable.")).Last("mov [0x*], ?l").Target(TEXT("GIsServer"));

bool& GIsGameThreadIdInitialized = *(bool*)Scanner::String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms")).Start().Find("cmp byte ptr [0x*], 0x0").Target(TEXT("GIsGameThreadIdInitialized"));
uint32& GGameThreadId = *(uint32*)Scanner::String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms")).Start().Find("cmp byte ptr [0x*], 0x0").Find("cmp *, [0x*]").Target(TEXT("GGameThreadId"));
uint32* GSlateLoadingThreadId = []
{
    auto GameThreadCheck = Scanner::String(TEXT("FDistanceFieldVolumeTextureAtlas::UpdateAllocations took %.1fms")).Start().Find("cmp byte ptr [0x*], 0x0").Find("cmp *, [0x*]; jz *; cmp *, [0x*]");

    return GameThreadCheck.Get() ? (uint32*)GameThreadCheck.Find("cmp *, [0x*]").Target(TEXT("GSlateLoadingThreadId")) : nullptr;
}();

FOutputDeviceRedirector* GetGlobalLogSingleton()
{
    static auto GetGlobalLogSingleton = Scanner::Function<FOutputDeviceRedirector* (*)()>(Scanner::String(TEXT("NOCONSOLE")).Call(2).Get(TEXT("GetGlobalLogSingleton")));

    return GetGlobalLogSingleton();
}
