# ESP32 Time Synchronization via HTTP

This project demonstrates how to synchronize the system clock of an ESP32 microcontroller using an online time API over Wi-Fi.

The ESP32 connects to a wireless network, sends an HTTP GET request to a time service, extracts the Unix timestamp from the JSON response, and updates its system clock using `settimeofday()`.

## Features

- **Wi-Fi Connectivity:** Connects the ESP32 to a wireless network.
- **HTTP Communication:** Retrieves time information from an online API using an HTTP GET request.
- **JSON Data Parsing:** Extracts the `unixtime` value from the API response.
- **Unix Timestamp Conversion:** Uses the Unix timestamp to set the ESP32 system clock.
- **System Time Configuration:** Updates the system time using the standard `settimeofday()` function.
- **Serial Monitoring:** Displays connection status, the retrieved timestamp, and the synchronized date and time.
- **Error Handling:** Checks HTTP response codes and verifies that the timestamp and system time are available.

## Hardware Requirements

- ESP32 development board
- USB cable
- Wi-Fi network with internet access

## Software Requirements

- Arduino IDE
- ESP32 Arduino Core
- `WiFi.h`
- `HTTPClient.h`
- `time.h`

## How It Works

1. The ESP32 connects to the configured Wi-Fi network.
2. It sends an HTTP GET request to the online time API.
3. The response is checked for a successful HTTP status code (`200`).
4. The program extracts the `unixtime` field from the JSON response.
5. The Unix timestamp is used to update the ESP32 system clock with `settimeofday()`.
6. The synchronized date and time are displayed through the Serial Monitor.

## Configuration

Update the Wi-Fi credentials in the source code before uploading the firmware:

- `ssid`: Your Wi-Fi network name.
- `password`: Your Wi-Fi password.

The program also uses an online time API endpoint, which must be accessible from the network.

## Example Serial Output

```text
Connecting to WiFi...
WiFi connected!
IP: 192.168.1.xxx
Getting time...
HTTP code: 200
Unix time: 1792xxxxxx
ESP32 clock synchronized!
ESP32 UTC time: 2026-10-17 09:00:00
Time setup completed!
```

*The IP address, Unix timestamp, and displayed time will vary.*

## Potential Applications

Accurate system time is useful for embedded applications that require:

- Timestamped sensor readings
- Event logging
- Time-based automation
- Scheduling tasks
- Recording the time of image captures
- Generating timestamps for HTTP requests and other network operations

## Project Goal

The goal of this project is to explore Wi-Fi networking, HTTP communication, API integration, Unix timestamps, and system time management on ESP32-based embedded systems.

It provides a foundation for adding time-aware functionality to IoT devices without requiring a dedicated real-time clock (RTC) module.
