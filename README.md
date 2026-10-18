# Velora

Velora is a universal Fortnite gameserver. It turns a normal Fortnite client into a dedicated server that loads the map, accepts players and runs full Battle Royale matches.

Client builds ship without most of Epic's server code. Velora puts back only what is missing and leaves everything else to the game itself. Every rebuilt function follows Epic's own code as closely as possible, so matches play the way they did back then. Functions are found by what the code does (strings, calls, instructions), not by hardcoded byte patterns, so one DLL runs on every supported build.

This repository does not include Fortnite game files, a launcher or a backend.

> Velora is an independent project. It is not affiliated with, endorsed by or supported by Epic Games.

## Version support

| Fortnite version | Status |
| --- | --- |
| 1.7.2 (CL-3700114) | Playable, in testing |
| 1.8 (CL-3724489) | Playable, in testing |
| 1.10 (CL-3790078) | In development |

- **Playable** means a full match works from warmup to Victory Royale.
- **In development** means work has started but the build is not playable yet.
- The goal is every Battle Royale build up to Chapter 1 Season 7, one engine version at a time.

## Features

- Warmup island, Battle Bus, skydiving and gliding
- Storm with the build's own phases and timings
- Solo, Duo and Squad, knocked state and revive
- Building, editing and repairing
- Harvesting
- Chests, floor loot, ammo boxes and supply drops with the build's own loot tables
- Inventory, quickbars, picking up, dropping and swapping items
- Traps and jump pads
- Emotes (1.8 and later)
- Locker cosmetics (1.8 and later)
- Death, spectating, eliminations and Victory Royale
- Automatic restart for the next match

## Configuration

Basic settings are in [`Velora/Config.h`](Velora/Config.h):

| Setting | What it does |
| --- | --- |
| `Map` | Map that loads at startup and after every match |
| `Playlist` | `AthenaSolo`, `AthenaDuo` or `AthenaSquad` |
| `Port` | Port the server listens on |
| `WarmupRequiredPlayerCount` | Players needed before the warmup starts |
| `bRestartAfterMatch` | Reload the map after a match ends |
| `bConsole` | Show the log console window |

The playlist can also be set without rebuilding by adding `-playlistId=2` (Solo), `-playlistId=10` (Duo) or `-playlistId=9` (Squad) to the server's launch arguments.

## Building

### Requirements

- Windows 10 or 11
- Visual Studio 2022 with the Desktop development with C++ workload

### Steps

1. Clone the repository:

   ```
   git clone https://github.com/Jeremy1926/Velora
   ```

2. Open `Velora/Velora.sln` in Visual Studio.
3. Select **Release** and **x64**.
4. Build the solution. The output is in `Velora/x64/Release`.

## Usage

- `Velora.dll` goes into the server process.
- `VeloraClient.dll` goes into every client.

Most launchers have a slot for a game server DLL and one for a client or console DLL. Velora writes `Velora.log` next to the game executable, and the console window shows the same lines.

## Reporting issues

Please include:

1. Fortnite version and CL
2. Playlist and number of players
3. What you did and what happened
4. `Velora.log` and the server's game log
5. Screenshots if it is something you can see

## Credits

- [Zydis](https://github.com/zyantific/zydis) for instruction decoding
- [MinHook](https://github.com/TsudaKageyu/minhook) for hooking

## License

Velora is released under the [MIT License](LICENSE).
