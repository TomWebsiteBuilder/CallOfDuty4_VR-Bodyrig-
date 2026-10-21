# Call of Duty 4 VR — Experimental BodyRig Fork

An experimental single-player VR fork of
[jplakon/CallOfDuty4_VR](https://github.com/jplakon/CallOfDuty4_VR), built on
[KisakCOD](https://github.com/SwagSoftware/KisakCOD), for the original 2007
Call of Duty 4: Modern Warfare.

**This is a vibecoded work in progress.** I have been developing it with help
from Codex, Claude and Gemini, testing changes in VR and iterating on what
works. Expect rough edges and regressions. This is not a polished or fully
validated release, and passing automated tests does not mean every mission or
interaction works correctly in a headset.

## What this fork changes

- A custom 3D main menu and 3D theater presentation for cutscenes.
- A semi-3D pause menu for use in VR.
- Physically climbable ladders using motion-controller grabs.
- Physical body equipment: magazine holders, grenades, knife and weapon holsters.
- Tracked arms and gloves with procedural sleeve IK and mission-specific materials.
- Manual magazine reloads, weapon racking, grenade handling and two-hand interactions.
- Experimental campaign collision, scope, brightness and rendering adjustments.
- A weapons testing launcher for focused in-game checks.

The latest change brings the pouches closer to the player and hides the torso,
belt and chest-webbing mesh while retaining the arms, hands and equipment.
Its exact fit, overlap and interaction comfort still need headset verification.

Compatibility across platforms and runtimes needs further testing. Upstream
release downloads do not include this fork's BodyRig changes.

## Known issues

- **Frame drops:** performance can drop sharply in some scenes or interactions.
  Scope rendering has received changes, but performance problems are not resolved.
- **Strange gripping issues:** grabbing, releasing, two-hand handling and nearby
  equipment selection can behave unexpectedly. Hand/weapon alignment and
  attachment need further testing across weapons and missions.
- **Textures freaking out:** texture/material corruption, flickering or other
  visual glitches can still occur. The cause is not fully established and this
  fork does not claim a universal fix.
- **Incomplete campaign validation:** collision, scripted sequences, equipment
  handling and long-session stability still need testing. Historical handoff
  notes contain fixes and test claims from earlier iterations; they are not a
  guarantee that all related symptoms are gone.

See [KNOWN-ISSUES.md](KNOWN-ISSUES.md) for the fork status and inherited issues.
When reporting a bug, include your commit, headset/runtime, GPU, mission and
checkpoint, reproduction steps and relevant logs. A short video helps with
visual or gripping problems.

## Testing

Automated checks cover the configurator, shaders, grenade interactions, ladders
and world-canvas behavior. The latest torso and pouch changes still require
in-headset acceptance testing.

See [the upstream VR documentation](docs/UPSTREAM-VR-README.md) for general
project information.

## Install on Windows

**This BodyRig fork currently has no prebuilt installer or portable release.**
To play this version on Windows, build it from source using the steps below.
GitHub's **Code → Download ZIP** is source code, not a playable mod package.
Upstream releases do not contain this fork's BodyRig changes. The guided Setup
and ZIP instructions in [INSTALL.md](INSTALL.md) describe upstream packages;
use the manual steps here for this fork.

### 1. Prepare the game and build tools

- Install your legitimate **original 2007 Call of Duty 4: Modern Warfare** and
  launch single-player once. Modern Warfare Remastered is not compatible.
- Copy the complete game folder into a separate, writable folder for testing.
  It must contain `iw3sp.exe`, `localization.txt`, `main` and `zone`.
  Apply the mod to this copy so the original installation stays intact.
- Install Git for Windows, CMake, and **Visual Studio 2022** (or its Build Tools)
  with **Desktop development with C++**, the MSVC v143 x86/x64 tools and a
  Windows SDK.
- Install the [DirectX SDK (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
  for the legacy build headers and libraries. Open a new terminal after
  installation so its `DXSDK_DIR` environment variable is available.
- Install the [DirectX End-User Runtimes (June 2010)](https://www.microsoft.com/en-us/download/details.aspx?id=8109)
  if the legacy runtime is missing. Extract the download and run `DXSETUP.exe`.
  A current DirectX version alone does not supply every legacy D3DX library.

### 2. Build the Windows executables

Open **Developer PowerShell for VS 2022** and run these commands from a folder
where you keep source projects:

```powershell
git clone --recurse-submodules https://github.com/TomWebsiteBuilder/CallOfDuty4_VR-Bodyrig-.git
cd CallOfDuty4_VR-Bodyrig-
Test-Path "$env:DXSDK_DIR/Include/d3dx9.h"
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release --target KisakCOD-sp KisakCOD-VR-Configurator KisakCOD-VR-Input-Mapper KisakCOD-VR-Test --parallel 8
```

The `Test-Path` check should return `True`. If it returns `False`, set
`$env:DXSDK_DIR` to the folder where you installed the June 2010 SDK before
configuring CMake. Use **Win32**, even on 64-bit Windows: the game and its
libraries are 32-bit. Wait for a successful Release build before continuing.
The executables are written to `bin\Release` in the source folder.

### 3. Copy the mod into your separate game folder

Using File Explorer, copy the following files from the source folder into the
root of your **game copy**, beside `iw3sp.exe`:

| Source | Destination in the game copy |
| --- | --- |
| `bin/Release/KisakCOD-sp.exe` | `KisakCOD-sp.exe` |
| `bin/Release/KisakCOD-VR-Configurator.exe` | `KisakCOD-VR-Configurator.exe` |
| `bin/Release/KisakCOD-VR-Input-Mapper.exe` | `KisakCOD-VR-Input-Mapper.exe` |
| `bin/Release/KisakCOD-VR-Test.exe` | `KisakCOD-VR-Test.exe` (optional testing launcher) |
| `deps/steamsdk/steam_api.dll` | `steam_api.dll` |
| `deps/binklib/binkw32.dll` | `binkw32.dll` |
| `deps/msslib/dlls/mss32.dll` | `mss32.dll` |
| Contents of `deps/msslib/dlls/miles/` | Merge into `miles/` |
| `release/package/Launch-KisakCOD-VR.bat` | `Launch-KisakCOD-VR.bat` |
| `release/package/Launch-KisakCOD-VR-Diagnostics.bat` | `Launch-KisakCOD-VR-Diagnostics.bat` |
| `release/package/VR-Settings.bat` | `VR-Settings.bat` |

Allow replacement of the matching runtime files **in the game copy**.
Keep `iw3sp.exe` under its original name. The mod uses your own installed game
assets; this repository does not include them.

### 4. Configure VR and launch

1. Connect your headset and start your VR runtime. For OpenXR, select a runtime
   that supports 32-bit applications. For the experimental OpenVR backend,
   start SteamVR first. See [runtime backend details](INSTALL.md#runtime-backend).
2. Run `KisakCOD-VR-Configurator.exe` from the game copy. Select your headset,
   controllers and runtime, then open **Setup & Compatibility** and rescan.
3. Resolve any **Blocked** items, save your settings, and use **Save & Launch**.
4. For later sessions, start the runtime and run `Launch-KisakCOD-VR.bat`.

If a DLL is missing or the headset stays blank, see
[common problems](INSTALL.md#common-problems). Windows headset compatibility
for this experimental fork still needs testing; the known issues above apply.

## Credits and licence

- Tom / [TomWebsiteBuilder](https://github.com/TomWebsiteBuilder): fork direction,
  VR testing and AI-assisted development.
- Codex (OpenAI), Claude (Anthropic) and Gemini (Google): AI development assistance.
  These are tool credits, not claims of vendor endorsement or human maintainership.
- jplakon and the CallOfDuty4_VR contributors: upstream VR conversion.
- SwagSoftware and the KisakCOD contributors: underlying project.
- Infinity Ward and the original Call of Duty 4 team; Khronos OpenXR and the
  other upstream dependencies and contributors.

Distributed under [GPL-3.0](LICENSE); see [third-party notices](THIRD-PARTY-NOTICES.md).
Call of Duty and its game assets belong to their respective owners. This is
an unofficial community project.
