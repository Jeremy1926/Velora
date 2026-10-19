# Velora

Universal gameserver for Fortnite.

[![Discord](https://img.shields.io/badge/Discord-Join%20the%20server-5865F2?style=for-the-badge&logo=discord&logoColor=white)](https://discord.gg/nrTrnQFa3W)

> Not affiliated with Epic Games.

## Version support

| Season | Versions |
| --- | --- |
| Pre-Season | 1.7.2 |
| Season 1 | 1.8 to 1.10 |
| Season 2 | 1.11 to 2.5 |
| Season 3 | 3.0 to 3.6 |
| Season 4 | 4.0 to 4.5 |
| Season 5 | 5.00 to 5.41 |
| Season 6 | 6.00 |

If a build in these ranges doesn't work, [open an issue](https://github.com/Jeremy1926/Velora/issues).

## Features

- Warmup, Battle Bus, skydiving
- Storm
- Solo, Duo, Squad
- Knocked and revive
- Building, editing, repairing
- Harvesting
- Chests, floor loot, ammo boxes, supply drops
- Picking up, dropping, swapping
- Traps, jump pads
- Emotes (1.8+)
- Locker cosmetics (1.8+)
- Spectating, Victory Royale
- Restart after every match

## To-do

- XP and challenges
- Season 5 to 7

## Configuration

`Velora/Config.h`

| Option | Description |
| --- | --- |
| `Map` | Map to load |
| `Playlist` | Playlist asset path, e.g. `/Game/Athena/Playlists/Playlist_DefaultSolo.Playlist_DefaultSolo` |
| `Port` | Listen port |
| `WarmupRequiredPlayerCount` | Players needed to start warmup |
| `bRestartAfterMatch` | Restart when the match ends |
| `bConsole` | Open a console window |

## Building

Needs Visual Studio 2022 with Desktop development with C++.

1. `git clone https://github.com/Jeremy1926/Velora`
2. Open `Velora/Velora.sln`
3. Build Release x64

Output: `Velora/x64/Release`

## Usage

- `Velora.dll`: server
- `VeloraClient.dll`: clients
- Log: `Velora.log` next to the game exe

## Issues

Include:

- Fortnite version and CL
- Playlist and player count
- Steps to reproduce
- `Velora.log` and the game log
- Screenshots

## Credits

- [Zydis](https://github.com/zyantific/zydis)
- [MinHook](https://github.com/TsudaKageyu/minhook)

## License

[MIT](LICENSE)
