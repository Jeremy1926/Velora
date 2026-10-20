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

TSharedPtr<IAnalyticsProvider>& UFortGameInstance::GetServerAnalyticsProvider()
{
    static auto ServerAnalyticsProvider = Anchor::Offset(TEXT("UFortGameInstance::ServerAnalyticsProvider"))
        .String(TEXT("Changing GameSessionId from '%s' to '%s'"))
        .Find("cmp *, 0x1; jnz *")
        .Call()
        .Is("lea rax, [rcx+0x*]; ret")
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

    return *(TSharedPtr<IAnalyticsProvider>*)((uint8_t*)this + ServerAnalyticsProvider);
}

TSharedPtr<IAnalyticsProvider>* UFortGameInstance::GetServerEsportsAnalyticsProvider()
{
    static auto ServerEsportsAnalyticsProvider = Anchor::Offset(TEXT("UFortGameInstance::ServerEsportsAnalyticsProvider"))
        .String(TEXT("Changing GameSessionId from '%s' to '%s'"))
        .Find("cmp *, 0x1; jnz *")
        .SameCall(6, 2)
        .Call(5)
        .Is("lea rax, [rcx+0x*]; ret")
        .Optional()
        .Disp();

    return ServerEsportsAnalyticsProvider ? (TSharedPtr<IAnalyticsProvider>*)((uint8_t*)this + ServerEsportsAnalyticsProvider) : nullptr;
}
