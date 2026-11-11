#include "pch.h"
#include "Online/FortGameSession.h"
#include "Online/FortPlaylistManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "UObject/Class.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "FortGlobals.h"
#include "Config.h"
#include "Scanner.h"
#include "Detour.h"

PATCH(AFortGameSession, ValidatePlayer, Anchor::Patch()
    .String(TEXT("No reservation in game."))
    .Last("jnz 0x*")
    .FirstTo("jnz")
    .Jump(), Anchor::Patch()
    .String(TEXT("No reservation in game."))
    .Last("jnz 0x*")
    .Target()
    .Find("call 0x*; test al, al; jnz 0x*")
    .Find("jnz 0x*")
    .Jump(), Anchor::Patch()
    .String(TEXT("No reservation in game."))
    .Start()
    .Find("mov rdx, *; mov rcx, *; call 0x*; test al, al; jnz 0x*; call 0x*")
    .Optional()
    .Find("jnz 0x*")
    .Jump())

void AFortGameSession::OpenMatchLevel(const UObject* WorldContextObject)
{
    TCHAR Playlist[256];
    wcscpy_s(Playlist, Config::Playlist);

    if (auto Arg = wcsstr(FCommandLine::Get(), TEXT("-playlist=")))
    {
        Arg = wcschr(Arg, '=') + 1;
        wcsncpy_s(Playlist, Arg, wcscspn(Arg, TEXT(" ")));
    }

    TCHAR Options[128] = {};
    FString Map;

    if (auto PlaylistClass = (UClass*)StaticFindObject(nullptr, nullptr, TEXT("/Script/FortniteGame.FortPlaylistAthena")))
    {
        static auto PreloadPersistentLevel = PlaylistClass->FindPropertyByName(FName(TEXT("PreloadPersistentLevel")));

        auto PlaylistData = Config::bReload && PreloadPersistentLevel ? StaticLoadObject(PlaylistClass, nullptr, TEXT("/BlastBerry/Playlists/Playlist_BlastBerrySquad.Playlist_BlastBerrySquad")) : nullptr;

        if (PlaylistData)
            Map = PreloadPersistentLevel->ContainerPtrToValuePtr<TSoftObjectPtr<UWorld>>(PlaylistData)->ToString();
        else
            PlaylistData = StaticLoadObject(PlaylistClass, nullptr, Playlist);

        if (PlaylistData)
        {
            auto Value = [&](const TCHAR* Name)
            {
                return *PlaylistClass->FindPropertyByName(FName(Name))->ContainerPtrToValuePtr<int32>(PlaylistData);
            };

            swprintf_s(Options, TEXT("playlistId=%d?MaxPlayers=%d?MaxTeamSize=%d"), Value(TEXT("PlaylistId")), Value(TEXT("MaxPlayers")), Value(TEXT("MaxTeamSize")));
        }
    }
    else
    {
        int32 PlaylistId = wcsstr(Playlist, TEXT("Playlist_DefaultDuo")) ? 10 : wcsstr(Playlist, TEXT("Playlist_DefaultSquad")) ? 9 : 2;
        int32 MaxTeamCount;
        int32 MaxTeamSize;
        int32 MaxSquadSize;
        int32 MaxPlayers;

        if (UFortPlaylistManager::GetMaxTeamInfoForPlaylist(nullptr, PlaylistId, MaxTeamCount, MaxTeamSize, MaxSquadSize, MaxPlayers))
            swprintf_s(Options, TEXT("playlistId=%d?MaxPlayers=%d?MaxTeamSize=%d"), PlaylistId, MaxPlayers, MaxTeamSize);
        else
            swprintf_s(Options, TEXT("playlistId=%d"), PlaylistId);
    }

    auto BRMapFullName = UFortGlobals::Get()->GetBRMapFullName();

    UGameplayStatics::OpenLevel(WorldContextObject, FName(!Map.IsEmpty() ? *Map : BRMapFullName ? **BRMapFullName : Config::Map), true, FString(Options));
}

void AFortGameSession::Restart()
{
    if (!Config::bRestartAfterMatch)
        return;

    OpenMatchLevel(this);
}
