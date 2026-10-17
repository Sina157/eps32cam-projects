## ESP32-CAM Image Capture and Cloud Transmission

This project uses an AI-Thinker ESP32-CAM to capture JPEG images and transmit them to cloud services over Wi-Fi using HTTPS.

The project focuses on reliable image transmission, memory management, and handling network communication on a resource-constrained embedded device.

### Features

- **Image Capture:** Captures images using the OV2640 camera module at SVGA resolution (800 × 600).
- **Camera Configuration:** Adjusts exposure, white balance, brightness, contrast, and sharpness to improve image quality.
- **Wi-Fi Connectivity:** Connects to a wireless network and monitors connection information, including signal strength (RSSI).
- **Dual Transmission Methods:** Attempts to send images through Google Apps Script first, then uses Cloudflare Workers as a fallback if the primary method fails.
- **Base64 Encoding:** Converts JPEG image data into Base64 for transmission to Google Apps Script.
- **Binary Image Transmission:** Sends the original JPEG data directly to Cloudflare Workers without Base64 encoding.
- **Chunked Data Transmission:** Splits image data into smaller chunks to manage network writes and improve transmission reliability.
- **PSRAM Utilization:** Uses external pseudo-static RAM (PSRAM) to allocate memory for the Base64-encoded image.
- **HTTPS Communication:** Uses secure client connections to communicate with cloud endpoints.

### Network Restrictions and Alternative Transmission Routes

One of the motivations behind this project is to investigate alternative network routes for transmitting images in environments where direct connections to certain services may be restricted or unreliable.

The project uses both Google Apps Script and Cloudflare Workers to test different HTTPS endpoints and transmission methods. If the primary transmission method reports a failure, the firmware attempts to use the alternative endpoint.

This design provides a practical way to experiment with network connectivity, cloud services, and fallback mechanisms under restricted network conditions.

The effectiveness of each route depends on the network environment, DNS resolution, TLS connectivity, and the accessibility of the target services.

### Why Chunked Transmission?

Sending image data in smaller chunks allows the program to control the size of individual network write operations instead of passing the entire payload to the client at once.

The implementation uses:
- **512-byte chunks** for Base64 data sent to Google Apps Script.
- **1024-byte chunks** for JPEG data sent to Cloudflare Workers.

This approach helps manage network writes and provides opportunities to handle failed write operations. It does not guarantee successful delivery by itself, so connection status and server responses remain important.

### Hardware Requirements

- AI-Thinker ESP32-CAM
- OV2640 camera module
- USB-to-serial adapter or compatible programmer
- Wi-Fi network

### Software and Libraries

- Arduino IDE
- ESP32 Arduino Core
- `esp_camera.h`
- `WiFi.h`
- `WiFiClientSecure.h`
- `mbedtls/base64.h`

### Configuration

Before uploading the firmware, configure the following values in the source code:

- Wi-Fi SSID and password
- Google Apps Script deployment URL
- Cloudflare Worker hostname

The cloud endpoints must be configured to accept the corresponding POST requests and process the received image data.

### Security Considerations

The current implementation uses `client.setInsecure()` for HTTPS connections, which disables server certificate verification. This is suitable only for controlled testing and should be replaced with proper certificate validation for production use.

Wi-Fi credentials and service URLs should also be kept out of public source code when they contain sensitive information.

### Project Goals

This project demonstrates practical experience with ESP32-based embedded development, camera configuration, Wi-Fi networking, HTTPS communication, Base64 encoding, PSRAM memory allocation, and fallback strategies for cloud data transmission.
