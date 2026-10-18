#pragma once

#include "UObject/Object.h"

enum class EFortAthenaPlaylist : uint8
{
    AthenaSolo,
    AthenaDuo,
    AthenaSquad,
    EFortAthenaPlaylist_MAX
};

class UFortPlaylistManager : public UObject
{
public:
    static bool GetMaxTeamInfoForPlaylist(int32 PlaylistId, int32& MaxTeamCount, int32& MaxTeamSize, int32& MaxSquadSize, int32& MaxPlayers);
};
