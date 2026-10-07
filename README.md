# WRG Robot Updater — China Normal

macOS 15+, Intel and Apple Silicon. No Arduino IDE, Python, libraries,
GitHub login or controller address entry is needed.

## First launch blocked by macOS

The ZIP includes `INSTALL-FIRST.txt` beside the app. This pilot is not
Apple-notarized. If macOS cannot verify the developer, try opening the app once,
then dismiss the warning. Only if you trust the download, go to **System Settings
→ Privacy & Security → Security → Open Anyway** for WRG Robot Updater and confirm
**Open** in the next prompt. macOS saves an exception for this app.
See [Apple's first-open instructions](https://support.apple.com/102445).

Do not disable Gatekeeper or remove security protections with Terminal commands.
If the warning says the app **will damage your computer** or **is damaged**, stop
and contact the team. Managed Macs may require administrator approval.

## Update your robot

1. Download `WRG-Robot-Updater-macOS.zip` from the latest Release. Extract it
   and move **WRG Robot Updater.app** to Applications.
2. Unplug the robot's motor battery. Connect by USB data cable. Close Serial
   Monitor and other apps using the port.
3. Open the app, select the USB port, and click **Check for updates**.
4. Confirm **Motor batteries unplugged**, then click **Update robot**.
5. Wait for green: firmware version and unchanged pairing verified after reboot.
   Do not use failed/stopped robots until repaired. Disconnect USB and test with
   lifted wheels before normal use; the app does not test movement or compass.

Pairing is read from the ESP32, never reassigned. Updates write only the
application at 0x10000, after checking chip identity and the actual partition
table. Unassigned robots, first-time setup, Taiwan and Special require the team
loader. Bundled/cached verified firmware works without internet.

**Stop operation** cancels downloads and force-stops uploads. Interrupting a
write may prevent booting; contact your team for recovery. Pairing storage is
not deliberately erased, but hardware faults can still occur.

## Pilot release limitations

Software tests pass; an end-to-end physical robot update and native Intel-Mac
operation have not yet been tested. The app is universal; the Intel upload tool
was smoke-tested using Rosetta on Apple Silicon.

The app is ad-hoc signed, **not Developer ID signed or Apple-notarized**. macOS
may block a downloaded copy's first launch. Friction-free broad distribution
is pending Developer ID signing and notarization. Do not disable system security
protections to run it. USB adapters or managed Macs may require drivers/approval.
The app includes both flashing tools and depends only on macOS system frameworks;
it does not require Arduino IDE, Python, Homebrew, or developer tools on the user's
Mac. We cannot yet promise installation-free operation for every USB adapter or
managed Mac. If no serial port appears, check the data cable and ask the team
whether that adapter needs its manufacturer's driver.

Only China Normal firmware is distributed here. No Taiwan/Special binaries,
team-password configuration, GitHub credentials or private signing keys are
included. The separate team loader retains password-protected version selection.

## Release verification and licenses

`manifest.json` is an Ed25519-signed envelope containing version, sequence,
chip, variant, partition hash, application hash and size. The public key is pinned
in the app and published as `update-public-key.txt`. The private signing key stays
in the maintainer's Mac Keychain, never in GitHub. A new trust key requires a new
trusted app, not an unverified download.

Third-party licenses are in `Contents/Resources/licenses`.
Unmodified upload tool/source: https://github.com/espressif/esptool/tree/v5.1.0.
Libraries: https://github.com/CodexPad/codex_pad_arduino_lib,
https://github.com/CodexPad/GamepadInput, https://github.com/h2zero/NimBLE-Arduino.
