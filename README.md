# Exoskeleton Neuro-Rehab OS

A knee and ankle rehabilitation prototype. An ESP32 reads an MPU6050 motion sensor strapped to the leg, drives a servo, and streams live data to a browser dashboard. The dashboard shows a 3D model of the leg, counts repetitions, tracks range of motion, and includes a game that is controlled by leg movement.

> **Prototype only.** This is a student research build. It is not a medical device and must not be used on a person without supervision from a qualified clinician, and never without the safety checks described below.

## Features

- **Live 3D biomechanical twin** that mirrors the sensor's angle in real time, with a choice of knee or ankle mapping
- **Repetition counter** and range-of-motion history chart, saved in the browser
- **Live gait metrics**: impact force (heel strike) and kinetic variance (swing or stance phase)
- **Neuro-Glider game**: hold a joint angle to fly through gates, with a high score and adjustable speed
- **Manual servo control** and motor speed delay, sent over WebSocket
- **Assist mode** that moves the servo in proportion to the measured angle, with an adjustable trigger threshold and maximum assist angle

## Hardware

| Part | Purpose |
|---|---|
| ESP32 DevKit (ESP-WROOM-32) | Microcontroller, Wi-Fi, WebSocket server |
| MPU6050 accelerometer and gyroscope | Measures leg angle and motion |
| Servo motor | Assists the joint |
| 5 V power supply for the servo | Powers the servo |
| Jumper wires | Connections |

## Wiring

See `hardware/circuit-diagram.jpg` for the full diagram.

| From | To ESP32 |
|---|---|
| MPU6050 VCC | 3V3 |
| MPU6050 GND | GND |
| MPU6050 SCL | D22 |
| MPU6050 SDA | D21 |
| Servo signal (yellow) | D18 |
| Servo power (red) | Vin |
| Servo ground (black) | GND |

## Software setup

1. **Install Arduino IDE** (version 2 or newer) and add the ESP32 board package from Espressif (Boards Manager, search "esp32").
2. **Install these libraries** (Library Manager):
   - WebSockets by Markus Sattler
   - ArduinoJson
   - ESP32Servo
   - Adafruit MPU6050 (this also installs Adafruit Unified Sensor)
3. **Set your Wi-Fi details** in `firmware/mainprojectfile.ino` (`ssid` and `password`). The ESP32 needs a 2.4 GHz network.
4. **Select the board** "ESP32 Dev Module" and the correct port, then upload the sketch.
5. **Open the Serial Monitor** at 115200 baud. Note the IP address shown after "Enter this IP in your HTML file".
6. **Open `web/index.html`** in Chrome or Edge. Set the IP address in the script so it matches the one from step 5, then reload the page.
7. The status light turns green when the dashboard is connected to the ESP32.

## How it works

- **Firmware**: reads the MPU6050 at about 30 Hz and sends JSON (`ax`, `ay`, `az`, `gx`, `gy`, `gz`) to all connected browsers. It also receives commands: `A:<angle>` sets the servo angle (0 to 180), and `S:<ms>` sets the delay between servo steps.
- **Angle**: pitch and roll come from the accelerometer, with an offset set by the Calibrate Zero button and a simple low-pass filter.
- **Repetitions**: one repetition is counted when the pitch rises above 30° and then falls below 10°.
- **Assist mode**: the servo angle follows the pitch multiplied by 1.5, capped at the selected maximum. A 5° hysteresis band prevents the motor from switching on and off at the threshold.

## Safety

Before any test with a person:
- Test with the leg fully unloaded and the servo disconnected from the joint.
- Start with the lowest assist power and the lowest speed.
- Keep a hand near the power switch at all times.
- The current firmware has no timeout: if the browser disconnects, the servo holds its last angle. Add a safe-stop timeout before real use.
- Have a qualified clinician supervise every session.

## Limitations

- The angle comes from accelerometer data only, so it is less accurate during fast movement.
- The IP address of the ESP32 must be set by hand in the dashboard.
- There is no encryption: the WebSocket connection is not secured, so use it only on a private network.
- The gait metrics are simple thresholds, not validated clinical measures.

## Project structure

```
firmware/mainprojectfile.ino     ESP32 firmware (sensor, servo, WebSocket server)
web/index.html                   live dashboard, 3D model, game and charts
hardware/circuit-diagram.jpg     wiring diagram
README.md
```

## Tech stack

ESP32 (Arduino), MPU6050, WebSockets, ArduinoJson, ESP32Servo. Browser: Three.js, Chart.js, Tailwind CSS.

## License

MIT. Add a `LICENSE` file with the MIT text before publishing.
