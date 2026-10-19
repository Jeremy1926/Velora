# Velora

Universal gameserver for Fortnite.

> Not affiliated with Epic Games.

## Version support

| Version | Status |
| --- | --- |
| 1.7.2 (CL-3700114) | Playable |
| 1.8 (CL-3724489) | Playable |
| 1.10 (CL-3790078) | Playable |
| 1.11 (CL-3807424) | Playable |
| 2.4.2 (CL-3870737) | Playable |
| 2.5.0 (CL-3889387) | Playable |
| 3.0 (CL-3901517) | Playable |
| 3.5 (CL-4008490) | Playable |
| 4.0 (CL-4039451) | Playable |

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
- Season 2 to 7

## Configuration

`Velora/Config.h`

| Option | Description |
| --- | --- |
| `Map` | Map to load |
| `Playlist` | `AthenaSolo`, `AthenaDuo` or `AthenaSquad` |
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
