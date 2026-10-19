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

## Build and run

You need Visual Studio 2022 C++ tools, CMake, the DirectX SDK/dependencies,
an OpenXR runtime, and your own legitimate original COD4 installation.

```bash
git clone --recurse-submodules https://github.com/TomWebsiteBuilder/CallOfDuty4_VR-Bodyrig-.git
cd CallOfDuty4_VR-Bodyrig-
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release --target KisakCOD-sp KisakCOD-VR-Configurator KisakCOD-VR-Input-Mapper KisakCOD-VR-Test --parallel 8
```

Executables are written to `bin/Release`. See [INSTALL.md](INSTALL.md),
and [upstream build instructions](docs/UPSTREAM-VR-README.md#build-from-source)
for setup details.
Use a separate experimental game directory. The repository contains source and
development documentation, not the original game's data or a ready-to-play
bundled game installation.

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
