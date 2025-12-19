# PN5180 NFC Reader - Fix Duplicated UID Issue

## 📋 Overview
A robust solution for reading NFC cards (ISO14443A) using the PN5180 module, specifically addressing a critical bug in standard libraries that causes duplicated UID readings. This implementation ensures each NFC card returns its correct, unique UID every time.
## ⚠️ The Problem: Duplicated UID Reading
When using standard PN5180 libraries, users encounter a critical bug:
- Different cards showing the same UID - Multiple distinct cards return identical UID values
- Same card showing different UIDs - A single card returns varying UIDs on consecutive reads
- Invalid UIDs - Readings of all zeros (0x00) or all ones (0xFF) that don't correspond to actual cards
## 🔍 Root Cause Analysis
The core problem lies in the standard PN5180 library's insufficient handling of the FIFO buffer between read operations:
- Insufficient FIFO buffer clearing - The readCardSerial() function doesn't reset the PN5180's internal FIFO buffer before new read operations
- Missing data validation - No proper validation of ATQA and SAK responses before processing UID data
- Incomplete error checking - Lack of validation for manufacturer codes and UID structure
## 🛠️ Solution: Enhanced Direct Buffer Processing
Instead of relying on the flawed readCardSerial() function, we implemented a direct approach using activateTypeA() with comprehensive validation:
Key Improvements:
- Direct FIFO Buffer Management
- Comprehensive Data Validation (ATQA, SAK, UID structure)
- Enhanced Error Detection with specific error codes
- Periodic Hardware Reset to ensure clean state broadcasting.
### File Structure
- PN5180-NFC-Reader/
- │
- ├── src/
- │   └── PN5180_reader.ino  # Main
- ├── inc/
- │   └──PN5180 Library        # Library for PN5180
- └── README.md                # This file
## 🚀 Quick Start
Hardware Requirements
- PN5180 NFC Reader Module
- ESP32 Development Board (or Arduino with sufficient pins)
- NFC Cards/Tags (ISO14443A compliant: Mifare Classic, Ultralight, NTAG)

Wiring Diagram
PN5180 Pin	  ESP32 Pin
NSS	          GPIO 16
BUSY	        GPIO 5
RST	          GPIO 17
SCK	          GPIO 18
MOSI	        GPIO 23
MISO	        GPIO 19
VCC	          3.3V
GND	          GND
- **Note**: Never swap NSS pin (16) and Busy pin (5)
## Software Installation
Install Arduino IDE (version 1.8.x or later)
Install PN5180 Library via Arduino Library Manager:
- Search for "PN5180" and install by tueddy (link: https://github.com/tueddy/PN5180-Library/blob/master/PN5180.h)
- Clone or download this repository
Open PN5180_reader.ino in Arduino IDE
Select board (ESP32 Dev Module) and correct COM port
Upload the sketch

## Contributing

Contributions are welcome! Please open an issue or submit a pull request.
