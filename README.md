# Smart Shopping Cart

This Smart Shopping Cart project integrates ESP32 hardware, RFID scanning, weight measurement, and WiFi data transmission, with a full-stack dashboard system for real-time analysis. It is an efficient prototype for intelligent retail automation.

---

## 🛠  Requirements

- **Arduino IDE** (ESP32 board package installed)  
- **Python 3.x** for backend (Flask + SQLite)  
- **Git** for version control  

---

##   Hardware Components

| Component            | Description                           |
|----------------------|---------------------------------------|
| ESP32 (FireBeetle-E) | Main microcontroller with WiFi        |
| MFRC522 (I2C)        | RFID scanner module                   |
| HX711                | Load cell amplifier module            |
| 1kg Beam Load Cell   | Measures weight of items              |
| 1602 LCD             | Display via LiquidCrystal library     |
| Tactile Buttons x2   | For checkout and reset                |
| LEDs (Green, Blue)   | Indicate scan & payment status        |
| Power Supply / USB   | ESP32 power                           |

---

##   Project Structure

| File/Folder            | Description                                                                 |
|------------------------|-----------------------------------------------------------------------------|
| `style.css/`           | Custom CSS styles for the dashboard UI                                      |
| `templates/`           | HTML templates for rendering Flask pages (`dashboard.html`)                 |
| `venv/`                | Python virtual environment with project dependencies                        |
| `app.py`               | Flask backend server handling API routes and rendering frontend             |
| `database.db`          | SQLite database storing product and transaction data                        |
| `IOT_WIFI.ino`         | Arduino sketch for ESP32: handles RFID, weight, display, and communication  |
| `mock_scanned_data.js`| JS file with mock data for chart testing                                     |
| `README.txt`           | (Deprecated) Initial readme file                                             |
| `requirements.txt`     | Python dependencies list for pip installation                               |
| `send_test_data.py`   | Script to send fake POST requests for backend testing                        |

---

##   Wiring Summary

| Device             | ESP32 Pin |
|--------------------|-----------|
| MFRC522 SDA        | GPIO 21   |
| MFRC522 SCL        | GPIO 22   |
| MFRC522 RST        | GPIO 15   |
| HX711 DT           | GPIO 39   |
| HX711 SCK          | GPIO 16   |
| LCD RS, E, D4–D7   | GPIO 4, 2, 25, 26, 18, 17 |
| Checkout Button    | GPIO 19   |
| Reset Button       | GPIO 23   |
| Green LED          | GPIO 13   |
| Blue LED           | GPIO 12   |

---

##   Arduino Libraries & Versions

| Library            | Version     |
|--------------------|-------------|
| MFRC522_I2C        | 1.0.0 (by miguelbalboa) |
| LiquidCrystal      | 1.0.7       |
| WiFi.h             | (built-in)  |
| HTTPClient.h       | (built-in)  |
| HX711.h            | 0.7.5 (by bogde) |
| ArduinoJson.h      | 6.21.2      |

---

##  SmartCart Website Setup Guide

### Backend Setup

1. **Clone or unzip** the project folder.
2. Open terminal and navigate to the project directory.
3. Set up the Python environment:
   ```bash
   python -m venv venv
   # On Windows:
   venv\Scripts\activate
   # On macOS/Linux:
   source venv/bin/activate
   pip install -r requirements.txt
   python app.py
4. The backend runs at: http://127.0.0.1:5000
5. Access dashboard: http://127.0.0.1:5000/dashboard
6. Simulate Without ESP32: python send_test_data.py
7. Real-Time Dashboard Features:
   Auto-refresh every 10 seconds
   Filter by customer/date
   Export CSV/PDF reports


##   Step-by-Step Operation

### 1.  Power On
- ESP32 initializes components (LCD, RFID, HX711).
- Connects to WiFi.
- Sets time via NTP.

### 2.  RFID Scan
- User scans RFID tag.
- ESP32 sends `GET` request to: `/get_product/<uid>`.
- Product name and price are displayed on LCD.

### 3.  Place Item on Scale
- **If Fruit**: Calculate price = weight × unit price.
- **If Non-Fruit**: Use fixed price.
- Green LED blinks to confirm.

### 4.  Scan More Items
- Repeat steps 2–3 for additional products.

### 5.  Checkout
- Press the **Checkout** button.
- Display total price.
- Scan RFID "payment card" (with specific UID).
- If valid:
  - Send cart data via `POST` to `/add_cart_data`.
  - Blue LED blinks and LCD displays "Thanks".

### 6.  Reset
- Press the **Reset** button.
- Clears cart and resets display.

---

##   Local Network Access

- Ensure **ESP32** and **Backend PC** are connected to the **same WiFi network**.
