# ☀️ Solar-Powered IoT Environmental Monitoring Station

> A solar-powered embedded monitoring system built around the ESP8266 for environmental sensing, gas/smoke indication, electrical telemetry, local visualization, and cloud-based monitoring.

---

## 📌 Overview

This project is an IoT-enabled environmental monitoring station designed around an ESP8266 NodeMCU.

The system combines multiple sensors and subsystems to monitor environmental conditions, gas/smoke sensor response, electrical parameters, and wireless connectivity.

Measurements are displayed locally through a 128×64 OLED and transmitted over Wi-Fi to ThingSpeak for remote monitoring.

The system is designed around a solar-powered battery architecture, allowing it to operate as a standalone monitoring platform.

---

## ✨ Key Features

- 🌡️ Temperature monitoring
- 💧 Humidity monitoring
- 🌬️ Atmospheric pressure monitoring
- 🫁 MQ-2 gas/smoke indication
- ⚡ Bus voltage measurement
- 🔋 Current measurement
- 🔌 Power consumption measurement
- 📺 128×64 OLED local display
- 👋 IR-based wave-to-wake interface
- 📡 ESP8266 Wi-Fi connectivity
- ☁️ ThingSpeak cloud telemetry
- ☀️ Solar-powered energy system
- 🔋 18650 Li-ion battery storage
- ⚙️ Custom voltage conversion and power architecture

---

## 🧩 System Architecture

```text
                    ☀️ SOLAR PANEL
                          │
                          ▼
                    ┌───────────┐
                    │  TP4056   │
                    │  Charger  │
                    └─────┬─────┘
                          │
                          ▼
                   🔋 18650 BATTERY
                          │
                          ▼
                    ┌───────────┐
                    │  INA219   │
                    │  Power    │
                    │ Monitoring│
                    └─────┬─────┘
                          │
                          ▼
                    ┌───────────┐
                    │  MT3608   │
                    │  Boost    │
                    └─────┬─────┘
                          │
                          ▼
                    ┌───────────┐
                    │  ESP8266  │
                    │   NodeMCU │
                    └─────┬─────┘
                          │
             ┌────────────┼────────────┐
             │            │            │
             ▼            ▼            ▼
          BME280        MQ-2         OLED
             │                         ▲
             │                         │
             │                       IR Sensor
             │
             └───────────┐
                         │
                         ▼
                    Wi-Fi Network
                         │
                         ▼
                    ThingSpeak
                    Cloud Platform
