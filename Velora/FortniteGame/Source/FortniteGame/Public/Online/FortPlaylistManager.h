#pragma once

#include "UObject/Object.h"

class UFortPlaylistManager : public UObject
{
public:
    static bool GetMaxTeamInfoForPlaylist(UFortPlaylistManager* PlaylistManager, int32 PlaylistId, int32& MaxTeamCount, int32& MaxTeamSize, int32& MaxSquadSize, int32& MaxPlayers);

    UObject* GetAthenaPlaylist(FName PlaylistName);
};
