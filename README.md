# PN5180 NFC Reader – Fix Duplicated UID Issue

## 📋 Overview
A robust solution for reading NFC cards (**ISO14443A**) using the **PN5180** module, specifically addressing a critical bug in standard libraries that causes **duplicated UID readings**.

This implementation ensures that each NFC card always returns its **correct, unique UID**.

---

## ⚠️ The Problem: Duplicated UID Reading
When using standard PN5180 libraries, users often encounter critical issues:

- **Different cards showing the same UID**  
  Multiple distinct cards return identical UID values

- **Same card showing different UIDs**  
  A single card returns varying UIDs on consecutive reads

- **Invalid UIDs**  
  Readings of all zeros (`0x00`) or all ones (`0xFF`) that do not correspond to real cards

---

## 🔍 Root Cause Analysis
The core problem lies in the standard PN5180 library’s insufficient handling of internal buffers and validation:

- **Insufficient FIFO buffer clearing**  
  `readCardSerial()` does not properly reset the PN5180 internal FIFO before new reads

- **Missing data validation**  
  No validation of ATQA and SAK responses before processing UID data

- **Incomplete error checking**  
  Manufacturer code and UID structure are not verified, allowing corrupted data

---

## 🛠️ Solution: Enhanced Direct Buffer Processing
Instead of relying on the flawed `readCardSerial()` function, this project uses a **direct approach** based on `activateTypeA()` with strict validation.

### Key Improvements
- Direct FIFO buffer management
- Comprehensive ATQA and SAK validation
- UID structure and manufacturer code verification
- Enhanced error detection with explicit status handling
- Periodic hardware reset to ensure a clean PN5180 state

---

## 📁 File Structure
- PN5180-NFC-Reader/
- ├── src/
- │ └── PN5180_reader.ino # Main application
- ├── inc/
- │ └── PN5180/ # PN5180 library
- └── README.md # This file

---

## 🚀 Quick Start

### 🧱 Hardware Requirements
- PN5180 NFC Reader Module
- ESP32 Development Board (or Arduino with sufficient GPIOs)
- ISO14443A NFC Cards/Tags, including:
  - MIFARE Classic
  - MIFARE Ultralight
  - NTAG series

---

## 🔧 Wiring Diagram

### PN5180 → ESP32

| PN5180 Pin | ESP32 Pin |
|-----------|-----------|
| NSS       | GPIO 16   |
| BUSY      | GPIO 5    |
| RST       | GPIO 17   |
| SCK       | GPIO 18   |
| MOSI      | GPIO 23   |
| MISO      | GPIO 19   |
| VCC       | 3.3V      |
| GND       | GND       |

> ⚠ **Note:**  
> **Never swap NSS (GPIO 16) and BUSY (GPIO 5)** – this will cause PN5180 communication failure.

![ESP32 PN5180 Wiring](https://github.com/user-attachments/assets/7ba69ed8-1a05-4073-891c-9e414e22d4fa)

---

## 💻 Software Installation

### 1️⃣ Arduino IDE
- Install **Arduino IDE 1.8.x or later**
- Install **ESP32 Board Support Package**

---

### 2️⃣ PN5180 Library
- Install PN5180 library by **tueddy**  
  🔗 https://github.com/tueddy/PN5180-Library

- Add library via: Sketch → Include Library → Add .ZIP Library

---

### 3️⃣ Build & Upload
1. Clone or download this repository
2. Open `PN5180_reader.ino` in Arduino IDE
3. Select:
   - Board: **ESP32 Dev Module**
   - Correct **COM Port**
4. Upload the sketch

---

## 🤝 Contributing
Contributions are welcome!  
Please open an **issue** or submit a **pull request**.
