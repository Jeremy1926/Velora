#include "pch.h"
#include "Online/FortPlaylistManager.h"
#include "FortGameMode.h"
#include "UObject/Class.h"
#include "Scanner.h"

bool UFortPlaylistManager::GetMaxTeamInfoForPlaylist(UFortPlaylistManager* PlaylistManager, int32 PlaylistId, int32& MaxTeamCount, int32& MaxTeamSize, int32& MaxSquadSize, int32& MaxPlayers)
{
    static auto bOwnedByGameMode = AFortGameMode::StaticClass()->FindPropertyByName(FName(TEXT("PlaylistManager"))) != nullptr;

    if (bOwnedByGameMode && !PlaylistManager)
        return false;

    static auto GetMaxTeamInfoForPlaylist = Anchor::Function<bool (*)(UFortPlaylistManager*, int32, int32*, int32*, int32*, int32*)>(TEXT("UFortPlaylistManager::GetMaxTeamInfoForPlaylist"))
        .String(TEXT("GetMaxTeamInfoForPlaylist: Unable to find playlist %d"))
        .Start()
        .Get();

    return GetMaxTeamInfoForPlaylist(PlaylistManager, PlaylistId, &MaxTeamCount, &MaxTeamSize, &MaxSquadSize, &MaxPlayers);
}

UObject* UFortPlaylistManager::GetAthenaPlaylist(FName PlaylistName)
{
    static auto GetAthenaPlaylist = Anchor::Function<UObject* (*)(UFortPlaylistManager*, FName)>(TEXT("UFortPlaylistManager::GetAthenaPlaylist"))
        .Smallest(TEXT("UFortPlaylistManager::GetAthenaPlaylist: Unable to find playlist %s"))
        .Start()
        .Or()
        .Smallest(TEXT("UFortPlaylistManager::GetPlaylist: Unable to find playlist %s"))
        .Start()
        .Get();

    return GetAthenaPlaylist(this, PlaylistName);
}
