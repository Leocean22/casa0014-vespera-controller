# Wiring Guide

This document describes how the components in the Vespera controller are connected.

## Module pin connections

The following connections can be inferred from the current firmware pin configuration:

### TCS3200 colour sensor

| TCS3200 pin | Arduino MKR WiFi 1010 pin | Function |
|---|---:|---|
| S0 | D2 | Output frequency scaling |
| S1 | D3 | Output frequency scaling |
| S2 | D4 | Colour filter selection |
| S3 | D5 | Colour filter selection |
| OUT | D6 | Sensor frequency output |
| LED | D13 | Turns the TCS3200 illumination LED on or off |

> **Note:** The TCS3200 module has two GND pins, but only one needs to be connected. Connect either GND pin to the Arduino GND rail.

### Rotary encoder

| Rotary encoder pin | Arduino MKR WiFi 1010 pin | Function |
|---|---:|---|
| CLK | D7 | Rotation signal |
| DT | D8 | Rotation direction signal |
| SW | D9 | Push-button input |

### WS2812B RGB LED

| LED pin | Arduino MKR WiFi 1010 pin | Function |
|---|---:|---|
| DIN | D10 | LED data input |

### DFRobot RGB LCD 1602

The LCD uses the I2C interface on the Arduino MKR WiFi 1010.

| LCD pin | Arduino MKR WiFi 1010 pin | Function |
|---|---|---|
| SDA | D11 / SDA | I2C data |
| SCL | D12 / SCL | I2C clock |

## Power and ground connections

Each module must be connected to the shared power rails:

- `VCC` → the Arduino 3.3 V power rail
- `GND` → the Arduino GND rail

All modules should share the same VCC and GND rails.

## Power supply options

The controller can be powered by USB or by a compatible 3.7 V rechargeable LiPo
battery connected to the Arduino MKR WiFi 1010 battery connector.
