# WRG Robot Updater

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
table. China Normal is the default. Team members use **Choose another version…**
and the team password to reveal the other versions and first-time setup controls.
Bundled/cached verified firmware works without internet.

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
The app includes both flashing tools and the Sparkle app-update framework;
it does not require Arduino IDE, Python, Homebrew, or developer tools on the user's
Mac. We cannot yet promise installation-free operation for every USB adapter or
managed Mac. If no serial port appears, check the data cable and ask the team
whether that adapter needs its manufacturer's driver.

This is one combined app for normal users and the team. China Normal firmware and
source are readable on GitHub. The other three firmware images and their source
archive are encrypted to hide their contents from casual browsing. The app contains
the decryption key and bundled firmware; this is obfuscation, not protection against
someone inspecting the app. The team password is a selection guard, not a GitHub
access control. Earlier plaintext versions remain in Git history; this change does
not make previously published code secret. No GitHub token or signing private key
is included in the app.

## Team versions in the same app

Use WRG Robot Updater 2.2.1 or later. China Normal remains the
default. Unlock other versions locally, select the matching board/version, then
click **Check for updates**. No GitHub account or token is needed. For assigned
robots, **Update keeping pairing** updates only the application and verifies
unchanged controller pairing after reboot. It refuses a different installed
variant; intentional version/carrier changes need the team's Flash & assign flow.
First-time setup uses **Check selected ports**, then **Flash & assign**, with each
controller address entered. That flow uses the latest verified application if
downloaded, otherwise the bundled/cached one.

China Normal Arduino source is under `firmware/chinaver_basic/`, with required libraries under `libraries/`.
Use the official Espressif Arduino core 3.3.6, ESP32 Dev Module, 4 MB default
partitions, DIO, 80 MHz. See `firmware/BUILD_RESULTS.md` for testing limitations.

## Updating the Mac app

Use **WRG Robot Updater → Check for app updates…** in the menu bar. Sparkle
checks the signed app feed and offers installation/relaunch when a newer app is
published. Robot operations are blocked during an app-update cycle; app checks
are refused while a robot operation is running. Dismiss the update dialog before
returning to flashing. App updates preserve pairing because they do not flash the
robot or overwrite the saved fleet map. Install the app in Applications first.
Sparkle can check periodically with your consent; it does not force installation.
Old apps without Sparkle need this combined app installed manually once.
Apple signing/notarization and a clean-Mac upgrade test are still needed before
claiming warning-free app updates on every Mac.

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
App updater: https://github.com/sparkle-project/Sparkle (2.10.0).
