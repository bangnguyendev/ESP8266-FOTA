
## Table of Contents
1. [Introduction 'src subfolder'](#src-subfolder)
3. [Installing with Boards Manager (More Information)](#installing-with-boards-manager-more-information)
4. [ESP8266 Firmware Over-the-Air (OTA) Update via HTTPS](#esp8266-firmware-over-the-air-ota-update-via-https)
5. [ESP8266 Firmware Verification CI/CD Action](#esp8266-firmware-verification-ci/cd-action)


## Setting up Sketch Building with Visual Studio Code
### Introduction
This guide demonstrates how to set up building Sketch with Visual Studio Code.

For detailed information, please refer to [Arduino CLI Sketch Specification](https://arduino.github.io/arduino-cli/0.35/sketch-specification/#src-subfolder) with focus on the "src subfolder" topic.

#### src subfolder
This would allow you to put the shared header files in a subfolder of your sketch folder:

```
MySketch
|_ MySketch.ino
|_ src
   |_ includes
      |_ myfile.h
```

Then you can do this in your sketch:

```cpp
#include "src/include/myfile.h"
```

The contents of the src subfolder are compiled recursively. Unlike the code files in the sketch root folder, these files are not shown as tabs in the IDEs.

This is useful for files you don't want to expose to the sketch user via the IDE's interface. It can be used to bundle libraries with the sketch in order to make it a self-contained project.

Arduino language files under the src folder are not supported.

In Arduino IDE 1.6.5-r5 and older, no recursive compilation was done.

In Arduino IDE 1.6.6 - 1.6.9, recursive compilation was done of all subfolders of the sketch folder.

In Arduino IDE 1.6.10 and newer, recursive compilation is limited to the src subfolder of the sketch folder.

## Installing with Boards Manager (More Information)

**Compatibility:** This guide covers installing third-party boards using Boards Manager, introduced in Arduino IDE versions 1.6.4 and later. 

**Preparation:**

1. **Download and install Arduino IDE:** Get the latest version compatible with your system (Windows, Mac OS, Linux 32-bit or 64-bit) from the official Arduino website: [https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE)

**Adding ESP8266 Board Package:**

1. **Open Arduino IDE Preferences:** Navigate to **File > Preferences** (or **Arduino > Preferences** on Mac).
2. **Add Boards Manager URL:** In the **Additional Boards Manager URLs** field, paste the following link:

   ```
   https://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```

   You can add multiple URLs separated by commas if needed for other board packages.
3. **Open Boards Manager:** Go to **Tools > Board > Boards Manager**.
4. **Install ESP8266 Platform:** Search for "esp8266" and click the **Install** button.
5. **Select ESP8266 Board (After Installation):** Once installed, choose your specific ESP8266 board from the **Tools > Board** menu.

**Latest Version:**

* Check the latest release version of the ESP8266 Arduino core: Latest release: [https://github.com/esp8266/Arduino/releases/latest/](https://github.com/esp8266/Arduino/releases/latest/)

**Documentation:**

* Refer to the ESP8266 Arduino core documentation for detailed instructions and examples: [invalid URL removed]

**Additional Libraries:**

This section provides links to popular libraries you might find useful for ESP8266 projects:

* **EEPROM ESP8266:** [https://github.com/esp8266/Arduino/blob/master/libraries/EEPROM/EEPROM.h](https://github.com/esp8266/Arduino/blob/master/libraries/EEPROM/EEPROM.h)
* **ArduinoJson:** [https://github.com/bblanchon/ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* **SmartConfig ESP:** [https://www.eeweb.com/smartconfig-how-to-turn-an-esp8266-into-a-smart-home-device/](https://www.eeweb.com/smartconfig-how-to-turn-an-esp8266-into-a-smart-home-device/)

**DigiCert Root Certificates:**

* While not directly related to ESP8266 development, this link provides DigiCert root certificates, which may be necessary for secure communication with certain servers: [https://www.digicert.com/kb/digicert-root-certificates.htm](https://www.digicert.com/kb/digicert-root-certificates.htm)

## ESP8266 Firmware Over-the-Air (OTA) Update via HTTPS

This project facilitates firmware updates for ESP8266 microcontroller units (MCUs) through HTTPS connections, comparing the firmware version with the `FirmwareVer` variable defined in the `Macro_define.h` file to initiate over-the-air (OTA) updates.

### Project Structure
```
├── include
│   └── Info_prod.json         // JSON file containing firmware version information
└── build
    └── main.ino.bin           // Binary firmware file
```

### MCU Boot Process
Upon startup, the ESP8266 microcontroller unit (MCU) follows a sequence of steps to ensure proper initialization and connectivity.

1. **WiFi Configuration Retrieval from EEPROM:**
   - The MCU retrieves WiFi network configuration details stored in the EEPROM memory. These details include the SSID and password necessary to connect to a WiFi network.

2. **Network Time Protocol (NTP) Synchronization:**
   - To synchronize its internal clock with a reliable time source, the MCU establishes a connection to an NTP server. This step is crucial for accurate timekeeping, which is essential for various time-sensitive operations, such as logging, scheduling, and encryption.

3. **HTTPS Connection to Server for Version Checking:**
   - Following successful NTP synchronization, the MCU initiates an HTTPS connection to a designated server. It retrieves version information from both a JSON file (`Info_prod.json`) and a binary firmware file (`main.ino.bin`) hosted on the server.
   - The MCU extracts the firmware version information from the JSON file to compare it with the locally stored firmware version (`FirmwareVer`) defined in the `Macro_define.h` file.
   - If a newer firmware version is detected, the MCU proceeds with the firmware update process.

### Firmware Update Workflow
Upon detecting a newer firmware version, the ESP8266 MCU engages in the OTA update process:

1. **Download of New Firmware Binary:**
   - The MCU retrieves the updated firmware binary file (`main.ino.bin`) from the server via the established HTTPS connection. This file contains the latest firmware version.

2. **Firmware Update Over-the-Air (OTA):**
   - Utilizing the downloaded binary file, the MCU performs an over-the-air (OTA) update to apply the new firmware version.
   - During the OTA update process, the MCU writes the updated firmware to its flash memory, replacing the existing firmware with the newer version.
   - Upon successful completion of the OTA update, the MCU reboots to activate the updated firmware.

### Security Considerations
- Secure communication protocols, such as HTTPS, are employed for all data exchanges, including WiFi configuration retrieval, NTP synchronization, and firmware version checking.
- Robust error handling mechanisms are implemented to manage potential failures during network operations and OTA updates.
- Validation and verification procedures are in place to ensure the integrity and authenticity of downloaded firmware binaries, mitigating the risk of tampering or malicious attacks.


## ESP8266 Firmware Verification CI/CD Action

### Introduction
This repository includes a Continuous Integration/Continuous Deployment (CI/CD) GitHub Action designed to verify the consistency of firmware versions between the `*.json` and `*.bin` files. This action ensures the accuracy of the binary firmware files (`*.bin`) relative to the firmware version specified in the JSON file (`Info_prod.json`). 

### Workflow Overview
The CI/CD workflow executes the following steps to validate firmware versions:

1. **Checkout Code:**
   - The workflow begins by checking out the source code from the repository.

2. **Install Dependencies:**
   - The necessary dependencies, such as `jq` for JSON processing, are installed to facilitate version extraction.

3. **Extract Firmware Versions:**
   - The action extracts the firmware versions from both the `Info_prod.json` file and the `Macro_define.h` header file.
   - The firmware version from the JSON file (`json_version`) and the firmware version from the header file (`header_version`) are captured.

4. **Compare Versions:**
   - The extracted firmware versions are compared to ensure consistency.
   - If the firmware versions match, the workflow proceeds with the validation process. Otherwise, an error is triggered, indicating a discrepancy between the versions.

### Purpose
The primary objective of this CI/CD action is to validate that the firmware version specified in the JSON file aligns with the firmware version embedded in the binary firmware files (`*.bin`). By performing this comparison, the action helps maintain version integrity and ensures that the correct firmware is deployed during OTA updates.

### Usage
To incorporate this CI/CD action into your development workflow, simply enable it within your GitHub repository. Upon each push to the `master` branch, the action will automatically execute, verifying the firmware versions and providing feedback on their consistency.

## Contributors
- bangnguyendev
- duybang140494@gmail.com
- https://bangnguyendev.github.io/

## License
None