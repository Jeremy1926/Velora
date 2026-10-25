#pragma once

#include "HAL/Platform.h"

namespace Config
{
    constexpr const TCHAR* Map = TEXT("/Game/Athena/Maps/Athena_Terrain");
    constexpr const TCHAR* Playlist = TEXT("/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo");
    constexpr int32 Port = 7777;
    constexpr int32 WarmupRequiredPlayerCount = 1;
    constexpr bool bRestartAfterMatch = true;
    constexpr bool bConsole = true;
    constexpr bool bGUI = true;
}
