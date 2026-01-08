📡 WiFi Assistant (ESP32)

A lightweight WiFi Assistant implemented on the ESP32, designed to make WiFi exploration and security monitoring simple.

This project provides three core functionalities:

🔍 Scan Nearby WiFi Networks – Lists all available SSIDs along with signal strength (RSSI), channel, and encryption type.

🌐 Create WLAN – Configures the ESP32 as an access point (AP) to host its own WiFi network.

🛡️ Detect Deauthentication Attacks – Monitors WiFi traffic in promiscuous mode to detect malicious deauth frames, a common denial-of-service attack on WiFi networks.

🚀 Features

ESP32-based standalone system (no external server required).

Real-time WiFi scanning and display.

AP mode for testing and extending local WiFi coverage.

Basic WiFi security monitoring and deauth attack detection.

Modular codebase – easy to extend with additional WiFi utilities.

🛠️ Requirements

Hardware: ESP32 Development Board

Software:

ESP-IDF or Arduino IDE

Serial monitor / logging tool (e.g., Arduino Serial Monitor, PuTTY, minicom)

