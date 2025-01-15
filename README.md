# RO Controller

**RO Controller** is an Arduino-based project designed to manage a reverse osmosis system. The controller automatically regulates water supply, cleans filters, and monitors the tank status.

## Features

- **Pressure Sensors**:
  - Monitor the availability of incoming water and tank fullness.
- **Automatic Valve Control**:
  - Manage water input, flushing, and draining.
- **Status Indication**:
  - LEDs display the current operating mode (filtering, flushing, water status).
- **Periodic Filter Flushing**:
  - Configurable flushing intervals: 6, 12, or 24 hours.

## Components and Pin Configuration

- **Input Sensors**:
  - Water availability sensor: pin `12`.
  - Tank level sensor: pin `11`.
- **Outputs for Control**:
  - Water input valve: pin `2`.
  - Flushing valve: pin `4`.
  - Pump: pin `3`.
  - Drain valve: pin `9`.
- **Indicators**:
  - Filtering LED: pin `6`.
  - Flushing LED: pin `7`.
  - Water status LED: pin `5`.
- **Flushing Button**:
  - Manual flushing activation button: pin `A2`.

## Configurable Parameters

- **Flushing Intervals**:
  - 6 hours, 12 hours, or 24 hours.
- **Flushing Duration**:
  - 15 seconds.

## How to Use

1. Connect components according to the specified pin configuration.
2. Upload the code to the Arduino controller (e.g., Arduino Pro Mini).
3. Power on the system to start automatic operation.
4. For manual flushing, press the button connected to pin `A2`.

## Author

The project is developed by **Maxim Pivovarov**.
2023
