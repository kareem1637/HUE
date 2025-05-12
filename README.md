# 🧪 Jaundice Detection System

A web-based system for jaundice detection using live video from an ESP32-CAM. The app captures face images, analyzes them using HSV color metrics, and visually displays the results.

---

## 📌 1. Project Overview

This system uses an **ESP32-CAM** module to stream live video. The user can capture a face image from the stream, and the backend uses **OpenCV** to process the image and detect jaundice signs based on **Hue, Saturation, Value (HSV)** color metrics.

---

## 🏗️ 2. System Architecture

### ➤ Frontend (Flask Web Interface)
- 📷 Live video feed
- 💾 Capture photo with custom filename
- 👁️ Show processed face (ROI), yellow mask, and HSV result

### ➤ Backend (Flask Server)
- Streams MJPEG feed from ESP32-CAM
- Captures and saves frames
- Applies image processing to:
  - Detect face
  - Extract ROI (under eyes)
  - Create a yellow region mask
  - Compute mean HSV values

### ➤ Hardware
- **ESP32-CAM** (streaming over HTTP)

---

## ✨ 3. Features

- 📷 Live MJPEG stream from ESP32-CAM
- 📝 Filename input for image capture
- 🤖 Face and ROI detection
- 🎨 Yellow mask overlay generation
- 🌈 HSV color analysis (mean values)
- 📊 Hue bar with cursor pointer

---

## 🧰 4. Technology Stack

| Component        | Technology         |
|------------------|--------------------|
| Frontend         | HTML, Bootstrap 5  |
| Backend          | Python, Flask      |
| Image Processing | OpenCV             |
| Hardware         | ESP32-CAM          |

---

## ⚙️ 5. Setup Instructions

### 🧲 Hardware Setup

#### Components:
- ESP32-CAM
- FTDI USB-to-Serial Adapter

#### Wiring:
```
GND  → GND  
IO0  → GND  (for flashing)  
TX   → RX  
RX   → TX  
5V   → VCC
```

#### Flashing the ESP32:
1. Flash `esp32-cam.ino` using Arduino IDE.
2. Disconnect IO0-GND jumper after upload.
3. Reconnect power to boot ESP32-CAM in normal mode.

---

### 💻 Software Setup

#### From Python:
1. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```
2. Connect to the ESP32-CAM Access Point:
   - **SSID:** `ESP32-CAM`
   - **Password:** `12345678`
3. Run the Flask server:
   ```bash
   python server.py
   ```
4. Open browser at: [http://192.168.4.2:5000](http://192.168.4.2:5000)

#### Or Using App:
- Run `app.exe` directly.
- Access the web UI at [http://192.168.4.2:5000](http://192.168.4.2:5000)

---

## 🔌 6. API Endpoints

| Endpoint              | Method | Description                          |
|-----------------------|--------|--------------------------------------|
| `/`                   | GET    | Homepage with video feed             |
| `/video_feed`         | GET    | Streams MJPEG video from ESP32-CAM   |
| `/trigger-capture`    | POST   | Captures current image frame         |
| `/check-result`       | GET    | Returns processed results in JSON    |
| `/get-image/<path>`   | GET    | Serves processed images (ROI/mask)   |

---

## 📁 7. File Structure

```
project/
│
├── static/
│   ├── css/
│   │   └── bootstrap.min.css
│   └── js/
│       └── bootstrap.bundle.min.js
│
├── templates/
│   └── index.html
│
├── server.py           # Flask backend
├── requirements.txt    # Python dependencies
└── app.exe             # Standalone app (optional)
```

---

## 🔄 8. Example Workflow

1. User sees a live video stream from ESP32-CAM.
2. Enters filename and clicks **Take Photo**.
3. Backend captures a frame and processes it:
   - Face detection
   - ROI extraction
   - Yellow mask creation
   - HSV value calculation
4. UI displays:
   - ROI image with facial overlay
   - Yellow mask visualization
   - HSV values (text)
   - Hue bar with pointer

---

## 📞 Contact

For feedback, queries, or collaboration:  
📧 *karim1637@gmail.com*  
---
