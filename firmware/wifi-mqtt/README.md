# WiFi and MQTT Controller

This folder contains the WiFi and MQTT firmware for the Vespera controller,
designed for the Arduino MKR WiFi 1010.

## Programs

| Program | Purpose |
|---|---|
| [Vespera Controller v2.0](vespera_controller_v2.0/) | Current controller with fixed-colour recognition |
| [Vespera Controller v1.0](vespera_controller_v1.0/) | Earlier controller with the original RGB recognition method |
| [TCS3200 Colour Test](TCS3200_color_test/) | Optional calibration firmware |

## v2.0 colour recognition

Compared with v1.0, v2.0 uses fixed ranges to recognise four colours, which is
more reliable for this project:

- 🔴 Red
- 🟢 Green
- 🔵 Blue
- 🟡 Yellow

Unknown readings are not assigned to a colour. More representative data can
improve calibration, but repeated data from the same conditions does not
guarantee greater accuracy.

## Optional calibration firmware

[TCS3200_color_test.ino](TCS3200_color_test/TCS3200_color_test.ino) is
provided for users who want to inspect the sensor readings or try their own
calibration. It prints the readings as comma-separated values for comparison.
