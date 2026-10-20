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
    static auto ServerAnalyticsProvider = []
    {
        auto GetServerAnalyticsProvider = Scanner::String(TEXT("Changing GameSessionId from '%s' to '%s'")).Find("cmp *, 0x1; jnz *").Call();

        if (GetServerAnalyticsProvider.Is("lea rax, [rcx+0x*]; ret"))
            return Scanner::Offset(TEXT("UFortGameInstance::ServerAnalyticsProvider"), (int64)GetServerAnalyticsProvider.Disp());

        auto EmptyGameState = Scanner::String(TEXT("%s|[EMPTY]"));
        auto Assign = Scanner::Reference((EmptyGameState.Get() ? EmptyGameState : Scanner::String("AnalyticsET")).Start().Get(), "call 0x*").Find("*");

        return Scanner::Offset(TEXT("UFortGameInstance::ServerAnalyticsProvider"), Assign.Imm() + Assign.Disp());
    }();

    return *(TSharedPtr<IAnalyticsProvider>*)((uint8_t*)this + ServerAnalyticsProvider);
}

TSharedPtr<IAnalyticsProvider>* UFortGameInstance::GetServerEsportsAnalyticsProvider()
{
    static auto ServerEsportsAnalyticsProvider = []
    {
        auto StoreGameSessionID = Scanner::String(TEXT("Changing GameSessionId from '%s' to '%s'")).Find("cmp *, 0x1; jnz *");
        auto GetServerEsportsAnalyticsProvider = StoreGameSessionID.Call(5);

        return GetServerEsportsAnalyticsProvider.Is("lea rax, [rcx+0x*]; ret") && StoreGameSessionID.Call(6).Get() == StoreGameSessionID.Call(2).Get() ? Scanner::Offset(TEXT("UFortGameInstance::ServerEsportsAnalyticsProvider"), GetServerEsportsAnalyticsProvider.Disp()) : 0;
    }();

    return ServerEsportsAnalyticsProvider ? (TSharedPtr<IAnalyticsProvider>*)((uint8_t*)this + ServerEsportsAnalyticsProvider) : nullptr;
}
