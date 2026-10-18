#include "pch.h"
#include "Online/FortPlaylistManager.h"
#include "Scanner.h"

bool UFortPlaylistManager::GetMaxTeamInfoForPlaylist(int32 PlaylistId, int32& MaxTeamCount, int32& MaxTeamSize, int32& MaxSquadSize, int32& MaxPlayers)
{
    static auto GetMaxTeamInfoForPlaylist = Scanner::Function<bool (*)(UFortPlaylistManager*, int32, int32*, int32*, int32*, int32*)>(Scanner::String(TEXT("GetMaxTeamInfoForPlaylist: Unable to find playlist %d")).Start().Get(TEXT("UFortPlaylistManager::GetMaxTeamInfoForPlaylist")));

    return GetMaxTeamInfoForPlaylist(nullptr, PlaylistId, &MaxTeamCount, &MaxTeamSize, &MaxSquadSize, &MaxPlayers);
}
