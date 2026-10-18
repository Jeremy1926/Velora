#include "pch.h"
#include "CoreGlobals.h"
#include "Scanner.h"

bool& GIsClient = *(bool*)Scanner::String(TEXT("Cannot run editor commandlet %s with game executable.")).Last("mov [0x*], al").Last("mov [0x*], al").Target();
bool& GIsServer = *(bool*)Scanner::String(TEXT("Cannot run editor commandlet %s with game executable.")).Last("mov [0x*], al").Target();

bool& GIsGameThreadIdInitialized = *(bool*)Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start().Find("cmp byte ptr [0x*], 0x0").Target();
uint32& GGameThreadId = *(uint32*)Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start().Find("cmp byte ptr [0x*], 0x0").Find("cmp *, [0x*]").Target();
uint32& GSlateLoadingThreadId = *(uint32*)Scanner::String(TEXT("UActorChannel::Close: ChIndex: %d, Actor: %s")).Start().Find("cmp byte ptr [0x*], 0x0").Find("cmp *, [0x*]", 2).Target();

FOutputDeviceRedirector* GetGlobalLogSingleton()
{
    static auto GetGlobalLogSingleton = (FOutputDeviceRedirector* (*)())Scanner::String(TEXT("NOCONSOLE")).Call(2).Get();

    return GetGlobalLogSingleton();
}
