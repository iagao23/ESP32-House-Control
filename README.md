# ESP32 Voice-Controlled Home Automation (TASMOTA Integration)

## 📝 Description
This project implements a voice-recognition interface using an **ESP32** and a **Digital Microphone (I2S)**. The system captures voice commands, interprets speech patterns, and triggers actions on a smart lamp running **TASMOTA** firmware via HTTP requests.

## 🎯 Objective
To provide a low-latency, localized voice control system that eliminates the need for external cloud assistants (like Alexa or Google Home) by interacting directly with TASMOTA-based devices over a local network.

---

## 🛠️ Hardware Requirements
- **Microcontroller:** ESP32 (NodeMCU or similar)
- **Audio Input:** Digital I2S Microphone (e.g., INMP441 or ICS-43434)
- **Output Device:** TASMOTA-powered Smart Lamp/Plug
- **Network:** Local Wi-Fi (2.4GHz)

## 📂 Project Architecture
```text
src/
├── main.cpp # Geral implementacion    
```
## 🚀 Technical Implementation
-Voice Interpretation

-The system uses [Specific Library, e.g., ESP-SR or TensorFlow Lite Micro] to process audio buffers in real-time. Once a keyword or command is identified, it triggers an event. 

-Tasmota Interaction

-Commands are sent using the TASMOTA HTTP API. 

-Target URL: http://<device_ip>/cm?cmnd=Power%20TOGGLE

## ⚡ Quick Start
- **Configure Hardware:** Connect the I2S microphone to the ESP32 (SCK, WS, SD pins).
- **Setup Wi-Fi:** Update your credentials in network.h.
- **Set Tasmota IP:** Enter the lamp's IP address in the configuration file.
- **Build & Flash:** Use [PlatformIO / ESP-IDF] to upload the firmware.
- **Operation:** Say the wake word to trigger the lamp.
