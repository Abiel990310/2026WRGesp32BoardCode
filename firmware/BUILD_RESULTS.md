# Verification — 2026-10-07

All four final sketches compiled successfully with `esp32:esp32:esp32`, official
Espressif core 3.3.6, and the libraries included in this package. No Bluepad32 core
was used. Default 4 MB partitions, DIO, 80 MHz flash, bootloader at 0x1000.

| Sketch | Flash bytes / 1,310,720 | RAM bytes / 327,680 |
| --- | ---: | ---: |
| taiwanboard_basic (Normal) | 656,827 | 36,760 |
| taiwanboard_full (Special) | 661,867 | 36,792 |
| chinaver_basic (Normal) | 656,827 | 36,760 |
| chinaver_full (Special) | 661,851 | 36,792 |

The `images/` files are these compiled builds. Both the old and new default
partition tables have NVS at 0x9000, length 0x5000; the loader does not write that
region or issue whole-chip erase. Its application uploads write bootloader,
partition table, boot_app0/OTA data, and application.

Host C++ tests passed for the actual shared configuration implementation:
address/nonce validation, wrong-chip and wrong-session rejection, RAM-only staging,
commit/readback, NVS-failure inhibition, oversized commands, reboot persistence,
and startup motion inhibition. These use mocked Serial/Preferences/ESP hardware.

Host C++ tests passed for the actual controller adapter with a mocked BLE vendor:
saved-MAC-only connection, initial input requirement, analog axis mapping, digital
triggers, existing debug UI button mapping, ten-minute quiet held state, malformed
packet rejection, state clearing on disconnect, stopping callbacks before delayed
reconnection, and suspension during provisioning.

Swift 6 tests passed for all four choices, 40-port uncapped selection, battery,
readiness and busy gates, duplicate and reserved-address blocking, strict nonce
reply parsing, and NVS-preserving flash commands. The native app builds for Intel and
Apple Silicon on macOS 15+ and its ad-hoc code signature was verified. The GUI's 40-port demo, normal
and special choices, and duplicate warning were checked without hardware writes.

**Not hardware-tested in this update:** USB flash + NVS assignment + reboot
readback on an actual robot, simultaneous multi-board uploads, motor polarity,
real controller disconnect timing, compass accuracy, and simultaneous controller
plus Mac BLE-debug connection. Start with one robot of each carrier, battery off
for provisioning, then lifted wheels for movement tests. No board was flashed.

## Separate public updater

The China-only public pilot is published at
https://github.com/Abiel990310/2026WRGesp32BoardCode/releases/tag/v1.0.0.
Its app version is 2.0.1; firmware version is 1.0.0. The public app bundles no
Taiwan/Special images, sketches, team verifier, or fleet records. Anonymous live
GitHub downloads, pinned Ed25519 signature verification, image size/hash checks,
and the Check for updates UI passed. The published ZIP digest matches the local ZIP.

Updates require an already assigned China Normal robot, verify its saved controller
and partition layout, write only the application at 0x10000, and check firmware
version and unchanged pairing after reboot. Host tests cover identity changes,
downgrades, malformed replies, and update safety gates. No physical robot update
was performed. Native Intel hardware remains untested. Both apps are ad-hoc signed,
not Developer ID signed or notarized; broad distribution still needs Apple signing.

## Compass disconnect hardening

All four actual compass implementations passed host tests with a mocked Wire bus:
held-low I2C with a bounded timeout, sensor removal, failed Euler read, failed
configuration write, fusion error, immediate invalidation, no repeated transactions
during held manual driving, retry exclusion during return/braking, stopped-only
recovery, and preservation of the captured zero. Recovery policy tests also cover
three-second backoff and millis rollover.

All four actual robot loops passed host tests with mocked PWM and controller/I2C:
manual drive continuing without compass, Square not trapping control, cancelling
return on sensor failure, and idle recovery. Both Special variants additionally
passed absolute-loss motor stop, switching to Manual, neutral re-arm before any
held absolute input can become manual throttle, refusing absolute entry without
compass, and never automatically re-entering absolute mode on sensor recovery.
The existing configuration and controller adapter regression tests still pass.
Physical unplugging and electrical/brownout faults are not hardware-tested.

## China motor polarity correction

China Normal and Special reverse M1, M2 and M3 relative to the preceding fleet
build. M4 and both Taiwan variants are unchanged. The China inversion flags are
now false, false, true, false for M1..M4. This follows the user's wheel test;
the corrected direction must be checked on the robot after flashing.

## Slightly slower driving turns

All four variants now cap manual steering at PWM 220 instead of 255. Special's
absolute-mode high-angle turns use the same 220 maximum, with the existing
near-target slowdown unchanged. Straight throttle remains 255; full steering
still pivots in place even with throttle held. Compass return remains PWM 170.
All four actual-loop host tests passed forward/reverse and both pivot-direction
checks; both Special variants passed the absolute-turn cap check. All four
images were rebuilt successfully. Physical turning speed is not measured here.

## Loader startup and cancellation

The model now defaults to China Normal on every launch. Stop kills the isolated
upload process group; a 750 ms escalation kills an unresponsive wrapper.
Nonblocking output reads check process lifetime, Stop and timeout without
waiting for pipe EOF. Busy state remains set until workers finish cleanup.
Host process tests cover normal output, TERM-ignoring parent/child shutdown,
bounded Stop completion, cancelled launch inhibition, retry after Stop, timeout,
task cancellation, wrapper escalation and six parallel stopped commands. Model
tests cover the China Normal default and existing fleet gates. No live board was
interrupted or flashed as part of this loader update.

## Public China selection and team gate

China Normal is the only public UI choice. A session-only team unlock exposes all
four choices; locking returns to China Normal. The execution/readiness gate also
rejects restricted firmware while locked, not just the picker. Host tests cover
missing password config, incorrect/empty passwords, all restricted choices,
correct unlock, busy-state exclusion, relocking and a fresh locked launch.
Passwords are represented by a salted 200,000-round PBKDF2-HMAC-SHA256 verifier;
the actual password is not saved in the app or package. This is not a private
firmware distribution mechanism: bundled files remain extractable.
The installed GUI was checked with the configured password: public China-only
selection, blank-password disabled button, incorrect-password message, successful
unlock, all four picker entries, Taiwan Special selection and return to locked
China Normal. The password sheet and final main window were visually inspected.
