# SH1106 OLED display support

The firmware driver supports a 128x32 SH1106 display connected over hardware I2C. The
device is configured through the normal `proto_Device` configuration, initialized and
updated through the firmware's `Device` lifecycle, and flushed only when its framebuffer
is dirty.

## Firmware configuration

`proto_OLEDDisplayDevice` in `proto/device.proto` contains:

- A required `I2CDevice` configuration (`block`, `sda`, `scl`, and `clock`).
- An optional 7-bit I2C address. When omitted, the driver uses `0x3C`.

Add the OLED device to the profile's device list to enable it. The display geometry is
fixed at 128x32; configurable dimensions and an `enabled` flag are intentionally omitted.
The device's presence in the configuration controls whether it is enabled.

The protocol schema and firmware factory are implemented here, but the external
Santroller Configurator UI must also expose the new device type before users can add or
edit it through that UI.

## Driver and rendering

`OLEDDisplay` implements `Device::begin`, `end`, `update`, and `using_pin`. It initializes
the SH1106 and writes a 512-byte page-addressed framebuffer using the controller's page and
column commands. The driver applies the two-column offset used by 128-pixel-wide SH1106
panels, and retains the dirty flag if a transfer fails so a later update can retry. Failed
initialization is retried at one-second intervals, and I2C failures are logged without
flooding the main loop.

The drawing API provides pixel, character, string, and status rendering helpers. The
status helper accepts a status string and player number; it is not yet connected to global
connection, profile, or player-state notifications. That state/event wiring is a separate
follow-up. The driver sends the display-off command when its device lifecycle ends; it
does not deinitialize the hardware I2C controller because that controller may be shared.

The implementation is compiled into the `santroller` target. `hardware_i2c` was already
linked by the firmware target, so no additional CMake library dependency is needed.
