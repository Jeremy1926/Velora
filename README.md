# Velora

Velora is a universal gameserver for Fortnite.

> Velora is not affiliated with Epic Games.

## Version support

| Fortnite version | Status |
| --- | --- |
| 1.7.2 (CL-3700114) | Playable, in testing |
| 1.8 (CL-3724489) | Playable, in testing |
| 1.10 (CL-3790078) | In development |

## Features

- Warmup island, Battle Bus, skydiving and gliding
- Storm
- Solo, Duo and Squad, knocked state and revive
- Building, editing and repairing
- Harvesting
- Chests, floor loot, ammo boxes and supply drops
- Picking up, dropping and swapping items
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

Velora writes `Velora.log` next to the game executable.

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
