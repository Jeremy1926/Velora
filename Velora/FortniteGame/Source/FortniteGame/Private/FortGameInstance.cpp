#include "pch.h"
#include "FortGameInstance.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"
#include "Scanner.h"

UFortPlaylistManager* UFortGameInstance::GetPlaylistManager() const
{
    static auto PlaylistManager = GetClass()->FindPropertyByName(FName(TEXT("PlaylistManager")));

    return *PlaylistManager->ContainerPtrToValuePtr<UFortPlaylistManager*>(this);
}

TSharedPtr<IAnalyticsProvider>* UFortGameInstance::GetServerAnalyticsProvider()
{
    static auto ServerAnalyticsProvider = Anchor::Offset(TEXT("UFortGameInstance::ServerAnalyticsProvider"))
        .String(TEXT("Changing GameSessionId from '%s' to '%s'"))
        .Find("cmp *, 0x1; jnz *")
        .Call()
        .Is("lea rax, [rcx+0x*]; ret")
        .Or()
        .String(TEXT("Changing GameSessionId from '%s' to '%s'"))
        .Find("cmp *, 0x1; jnz *")
        .Find("lea *, [*+0x*]")
        .Is("lea *, [*+0x*]; mov r8b, 0x1")
        .Or()
        .String(TEXT("Changing GameSessionId from '%s' to '%s'"))
        .Find("cmp *, 0x1; jnz *")
        .Find("lea *, [*+0x*]")
        .Is("lea *, [*+0x*]; mov r8b, r*")
        .Or()
        .String(TEXT("Changing GameSessionId from '%s' to '%s'"))
        .Find("cmp *, 0x1; jnz *")
        .Find("lea *, [*+0x*]")
        .Is("lea *, [*+0x*]; mov rcx, *; call 0x*")
        .Or()
        .String(TEXT("%s|[EMPTY]"))
        .Start()
        .Reference("call 0x*")
        .Find("*")
        .Or()
        .String("AnalyticsET")
        .Start()
        .Reference("call 0x*")
        .Find("*")
        .Sum();

    return ServerAnalyticsProvider > 0 ? (TSharedPtr<IAnalyticsProvider>*)((uint8_t*)this + ServerAnalyticsProvider) : nullptr;
}

TSharedPtr<IAnalyticsProvider>* UFortGameInstance::GetServerEsportsAnalyticsProvider()
{
    static auto ServerEsportsAnalyticsProvider = Anchor::Offset(TEXT("UFortGameInstance::ServerEsportsAnalyticsProvider"))
        .String(TEXT("Changing GameSessionId from '%s' to '%s'"))
        .Find("cmp *, 0x1; jnz *")
        .SameCall(6, 2)
        .Call(5)
        .Is("lea rax, [rcx+0x*]; ret")
        .Or()
        .String(TEXT("Changing GameSessionId from '%s' to '%s'"))
        .Find("cmp *, 0x1; jnz *")
        .Find("lea *, [*+0x*]", 2)
        .Is("lea *, [*+0x*]; xor r8d, r8d")
        .Optional()
        .Disp();

    return ServerEsportsAnalyticsProvider ? (TSharedPtr<IAnalyticsProvider>*)((uint8_t*)this + ServerEsportsAnalyticsProvider) : nullptr;
}
