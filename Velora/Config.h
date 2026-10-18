#pragma once

#include "Online/FortPlaylistManager.h"

namespace Config
{
    constexpr const TCHAR* Map = TEXT("/Game/Athena/Maps/Athena_Terrain");
    constexpr EFortAthenaPlaylist Playlist = EFortAthenaPlaylist::AthenaSolo;
    constexpr int32 Port = 7777;
    constexpr int32 WarmupRequiredPlayerCount = 1;
    constexpr bool bRestartAfterMatch = true;
    constexpr bool bConsole = false;
}
