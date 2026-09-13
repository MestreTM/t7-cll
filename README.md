# T7-CLL

Modified client by **MestreTM**.

Based on [Ezz BOIII](https://github.com/Ezz-lol/boiii-free), which is based on
the original BOIII work by [momo5502](https://github.com/momo5502) and
[X Labs](https://xlabs.dev/).

Thanks to Ezz, momo5502, X Labs, and the original BOIII authors.

You must legally own Call of Duty: Black Ops III.

---

## Layout

Put these next to `BlackOps3.exe`:

```text
<game>\
  BlackOps3.exe
  boiii.exe
  boiii\
    data\
      gamesettings\
      lookup_tables\
      scripts\
      ui_scripts\
    user\
```

`boiii\` is portable (next to the exe). The client does not use
`%LOCALAPPDATA%\boiii` and does not contact `r2.ezz.lol` unless you pass
`-online` or `-update`.

Copy `data\` from this repo into `boiii\data\`. Skip `data\launcher\`
(HTML launcher, unused).

| Path | Why |
|---|---|
| `data/lookup_tables/dvar_list.txt` | console / dvar names |
| `data/lookup_tables/hash_names.txt` | GSC hash names |
| `data/ui_scripts/` | T7 UI Lua |
| `data/scripts/` | GSC overrides |
| `data/gamesettings/` | playlist / game settings |

If `boiii\data` is empty, the exe looks for a `data\` folder next to the
exe or up the source tree and copies it (dropping `launcher\`).

Game folder fallbacks when `BlackOps3.exe` is not in the current directory:

1. `BO3_INSTALL` environment variable
2. `boiii\user\game_path.txt`
3. Steam library path for Black Ops III

---

## Launch

Double-click shows credits/help. After **OK**, the game starts (same as
`-launch`).

```text
boiii.exe
boiii.exe -launch
boiii.exe -dedicated
```

There is no flag that jumps straight into Multiplayer or Zombies. Both use
`BlackOps3.exe`; pick the mode in the T7 menu.

If the PE checksum of `BlackOps3.exe` does not match what this client
expects, it downloads a replacement from
`https://archive.org/download/t7_full_game/BlackOps3.exe`.
The previous file is renamed to `OldBlackOps3.exe`. If the download fails,
an English warning is shown and launch stops.

---

## Flags

### CLL

| Argument | Effect |
|---|---|
| `-launch` | Start without waiting on extra UI. Double-click already continues after OK |
| `-dedicated` | Use `BlackOps3_UnrankedDedicatedServer.exe` |
| `-about` | Credits dialog. In-game console: `about` |
| `-nick Name` / `-name Name` | Player name |
| `-online` | Contact `master.ezz.lol` (server list / friends). Allows Lua HTTP, Steam workshop scrape, public-IP lookup |
| `-nowatermark` / `-nobranding` | Hide only the in-game overlay. Window title and console prefix stay `CLL` |
| `-noconsole` | Do not allocate the external console |
| `-headless` | Print errors to stdout (no GUI error boxes) |
| `-update` | Allow replacing `boiii.exe` from the Ezz CDN (`r2.ezz.lol`) |
| `-noupdate` | Extra guard against the client updater |
| `-norelaunch` | Do not relaunch after a client update |

```text
boiii.exe
boiii.exe -launch -nick MestreTM -nowatermark -noconsole
boiii.exe -launch -online
boiii.exe -dedicated
```

### Display / audio

| Argument | Effect |
|---|---|
| `-windowed` | Windowed mode |
| `-borderless` | Borderless window |
| `-nointro` | Skip intro movies |
| `-nc` / `-nocinematics` | Skip cinematics |
| `-u` / `-uw` / `-ultrawide` | Ultrawide handling |
| `-nosnd` | No sound |

### Network / server

| Argument | Effect |
|---|---|
| `-port <n>` | Network port |
| `-noratelimit` | Disable rate limit |
| `-mitigatepacketspam` | Packet-spam mitigation |
| `-nosteam` | Skip Steam proxy where supported |

### Logs / debug

| Argument | Effect |
|---|---|
| `-d` / `-debug` / `-t` / `-trace` | Debug / trace logging |
| `-sct` / `-scr-trace` | Script tracing |
| `-vd` / `-vehicle-debug` | Vehicle debug |
| `-console` | Request the game console |
| `-trimlogs` | Trim log files |
| `-nologs` | Disable logs |
| `-fulllogs` | Verbose logs |
| `-ls` / `-log-script-errors` | Log script errors |
| `-quiet-crash` | Quieter crash reporting |
| `-dump` | Dump the loaded PE after load/unpack |

### Scripts / plugins / tools

| Argument | Effect |
|---|---|
| `-plugins` | Enable plugins |
| `-noplugins` | Disable plugins |
| `-unsafe-lua` | Allow unsafe Lua |
| `-e` / `-extract-assets` | Extract assets |
| `-o` / `-output <path>` | Output path for extract |
| `-ne` / `-no-ext` | Disable script extensions |
| `-c` / `-cheats` / `-enable-cheats` | Enable cheats |
| `-alias` | Alias helper |
| `-disable-loadlib` | Block `LoadLibrary` helper |
| `-keep-launcher` | Keep the original HTML launcher path (unused in CLL) |
| `-newsteamclient` | Alternate Steam client checksum path |
| `-safe` | Safe mode |

---

## Offline vs `-online`

Without `-online` the client stays local:

- no `r2.ezz.lol` client updater
- no `master.ezz.lol` / `m.ezz.lol`
- no `api.ipify.org`
- no Steam workshop HTTP
- Lua `game.httpget` / `game.httppost` return empty

Still used locally / when needed:

- `LoadLibrary` of `BlackOps3.exe`
- Archive.org fetch of `BlackOps3.exe` when the local checksum does not match
- joining a server you connect to (including that server's FastDL)
- Discord Rich Presence (local Discord IPC)

Steam Workshop download/UI is stubbed. Local `mods` / `usermaps` still work
if the files are already on disk.

---

## Branding

- Overlay: `CLL: <version>` (hidden with `-nowatermark`)
- Window title prefix: `CLL`
- Console prefix: `CLL>`
- Splash: stock image + white bold `Loading...` on the bottom right

---

## Build

Needs Visual Studio with **C++ Clang tools for Windows** (`msc-clangcl`).

```powershell
git clone --recursive https://github.com/MestreTM/t7-cll.git
cd t7-cll
git submodule update --init --recursive
.\build.bat
```

Output: `build\bin\x64\Release\boiii.exe`

Copy the exe **and** a `boiii\` folder (with `data`) into the game directory.

After changing sources, delete the PCH if MSBuild complains:

```powershell
Remove-Item -Recurse -Force .\build\obj\x64\Release\client -ErrorAction SilentlyContinue
.\build.bat
```

---

## Credits

- [Ezz / Ezz-lol/boiii-free](https://github.com/Ezz-lol/boiii-free)
- [momo5502](https://github.com/momo5502) and [X Labs](https://xlabs.dev/) — original BOIII
- Community T7 research and patches that BOIII built on
- **MestreTM** — CLL modifications (CLI, portable data, offline-by-default, branding)
