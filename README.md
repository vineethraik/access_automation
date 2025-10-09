# AccessOS

AccessOS is an ESP32-based project designed for fingerprint-based access control, featuring a web interface for user registration and login, and local button controls for various operations.

## Features

*   **Fingerprint Registration:** Register new fingerprints via a web interface with input validation for USN and Name.
    *   Name validation: 5-25 alphabetic characters only.
    *   USN validation: `4SFYYBB###` format (e.g., `4SF18EC124`), automatically converted to uppercase.
*   **Fingerprint Login:** Authenticate users via fingerprint scan.
*   **Web Interface:** A responsive web interface (`index.html`) served by the ESP32 for user interaction.
*   **WebSocket Communication:** Real-time communication between the ESP32 and the web interface for registration progress and login status.
*   **Local Controls:** Physical button interactions for registering new IDs, displaying device info, and clearing all fingerprint templates.
*   **EEPROM Storage:** Stores fingerprint template mappings and time counters in EEPROM.
*   **WiFi Connectivity:** Connects to an existing WiFi network and also creates an Access Point for initial configuration or direct connection.

## Technologies Used

*   **Microcontroller:** ESP32 (specifically `esp32doit-devkit-v1`)
*   **Framework:** Arduino Framework for ESP32
*   **PlatformIO:** Project management and build system.
*   **Libraries:**
    *   `Adafruit Fingerprint Sensor Library`: For interfacing with the fingerprint sensor.
    *   `ArduinoJson`: For handling JSON data in WebSocket communication.
    *   `WebSockets`: For WebSocket server functionality on the ESP32.
    *   `WebServer`: For serving the web interface.
    *   `DNSServer`: For DNS server functionality (likely for captive portal setup).
    *   `EEPROM`: For non-volatile storage of system data.
    *   `HTTPClient`: For making HTTP requests (if any, not explicitly seen in `main.cpp` but included).

## Setup Instructions

### Hardware

1.  **ESP32 Board:** `esp32doit-devkit-v1` or compatible ESP32 board.
2.  **Fingerprint Sensor:** Adafruit-compatible fingerprint sensor connected to `Serial2` (GPIO16, GPIO17).
3.  **Button:** A button connected to GPIO0 for local controls.
4.  **LED:** Built-in LED (usually GPIO2) for status indication.

### Software

1.  **PlatformIO IDE:** Install PlatformIO extension for VS Code or use PlatformIO CLI.
2.  **Project Dependencies:** The `platformio.ini` file specifies the required libraries. PlatformIO will automatically install them.

    ```ini
    [env:esp32doit-devkit-v1]
    platform = espressif32
    board = esp32doit-devkit-v1
    framework = arduino
    monitor_speed = 115200
    upload_speed = 921600
    lib_deps =
    	adafruit/Adafruit Fingerprint Sensor Library@^2.1.3
    	bblanchon/ArduinoJson@^7.4.2
    	links2004/WebSockets@^2.7.0
    ```

3.  **WiFi Credentials:** Update `src/credentials.h` with your WiFi network details and Access Point credentials.

    ```c++
    const char *WIFI_SSID = "YOUR_WIFI_SSID";
    const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
    const char *AP_SSID = "AccessOS"; // Default Access Point SSID
    const char *AP_PASSWORD = "";     // Default Access Point Password (can be empty)
    ```

### Deployment

1.  **Build:** Use PlatformIO to build the project.
2.  **Upload:** Upload the compiled firmware to your ESP32 board using PlatformIO.

## Usage

### Web Interface

1.  Connect to the ESP32's Access Point (`AccessOS`) or the configured WiFi network.
2.  Open a web browser and navigate to the ESP32's IP address (or `http://accessos.local` if mDNS is configured).
3.  **Register:**
    *   Click "Register".
    *   Enter USN (e.g., `4SF18EC124`) and Name (e.g., `JOHN DOE`).
    *   Click "Register Fingerprint" and follow the prompts on the fingerprint sensor.
4.  **Login:**
    *   Click "Login".
    *   Click "Login with Fingerprint" and scan your registered finger.

### Local Button Controls (GPIO0)

*   **1 Press:** Initiate new fingerprint registration.
*   **2 Presses:** Display fingerprint device information on the serial monitor.
*   **3 Presses:** Clear all registered fingerprint templates from the sensor and EEPROM.

## File Structure

*   `platformio.ini`: PlatformIO project configuration.
*   `src/`: Source code directory.
    *   `main.cpp`: Main Arduino sketch, `setup()` and `loop()` functions, initializes `AccessOS` and handles button presses.
    *   `AcessOS.h`: Defines the `AccessOS` class, encapsulating WiFi, WebServer, WebSockets, Fingerprint sensor, and EEPROM functionalities.
    *   `htmlData.h`: Generated header file containing the `index.html` content as a C++ string.
    *   `htmlToString.js`: Node.js script to convert `index.html` into `htmlData.h`.
    *   `index.html`: The web interface for AccessOS.
    *   `credentials.h`: Stores WiFi SSID/password and Access Point credentials.
    *   `timer.h`: Custom timer utility for event scheduling.
*   `include/`: Additional header files.
*   `lib/`: Private libraries.
*   `test/`: Test files.

## Contribution

Feel free to fork the repository, make improvements, and submit pull requests.
