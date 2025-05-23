Introduction
This Smart Shopping Cart project integrates ESP32 hardware, RFID scanning, weight measurement, and WiFi data transmission, with a full-stack dashboard system for real-time analysis. It is an efficient prototype for intelligent retail automation.

Requirements
Arduino IDE (ESP32 board package installed)
Python 3.x for backend (Flask + SQLite)
Git (for version control)

Component		            Description
ESP32 (FireBeetle-E)		Main microcontroller with WiFi
MFRC522 (I2C)		        RFID scanner module
HX711			              Load cell amplifier module
1kg Beam Load Cell		  Measures weight of items
1602 LCD			          For display via LiquidCrystal library
Tactile Buttons x2		  For checkout and reset
LEDs (Green, Blue)		  Indicate scan & payment status
Power Supply / USB		  ESP32 power  


Wiring Summary
Device			ESP32 Pin
MFRC522 SDA		GPIO 21
MFRC522 SCL		GPIO 22
MFRC522 RST		GPIO 15
HX711 DT		GPIO 39
HX711 SCK		GPIO 16
LCD RS, E, D4–D7		GPIO 4, 2, 25, 26, 18, 17
Checkout Button		GPIO 19
Reset Button		GPIO 23
Green LED		GPIO 13
Blue LED			GPIO 12


Folder or File		     Meaning
style.css/			       Contains custom CSS styles used to format the frontend dashboard.
templates/		         Stores HTML files for rendering frontend pages using Flask (e.g., dashboard.html).
venv/			             Python virtual environment folder containing isolated dependencies for the backend (created via python -m venv).
app.py			           The main Flask application backend. It handles API routes such as /add_cart_data and /get_data, manages the SQLite database, and serves the dashboard page.
database.db		         The local SQLite database storing transaction records and product information.
IOT_WIFI.ino		       Arduino (ESP32) sketch written in C++. It runs on the ESP32 board, handling RFID scanning, weight measurement, LCD display, WiFi communication, and sending purchase data to the backend.
mock_scanned_data.js	 JavaScript file containing mock scanned product count data, likely for frontend chart visualization or testing.
README.txt		         Basic documentation or instructions about the project, usually shown on GitHub. It provides a summary of the system and setup steps.
requirements.txt		   A list of Python dependencies and their versions used in the backend (e.g., Flask, requests). This is used to reinstall all required packages with pip install -r requirements.txt.
send_test_data.py		   A script that simulates POST requests to /add_cart_data for testing purposes. It generates and sends fake cart data to test backend functionality.


Required Arduino libraries	  Version
MFRC522_I2C          		      1.0.0（by miguelbalboa）
LiquidCrystal		              1.0.7
WiFi.h
HTTPClient.h
HX711.h			                  0.7.5（by bogde）
ArduinoJson.h		              6.21.2


SmartCart_Website Setup Guide
Backend Setup
1.Clone or unzip the SmartCart project folder.

2.Open a terminal in the project directory, then run the following:
python -m venv venv
venv\Scripts\activate       # (or source venv/bin/activate on macOS/Linux)
pip install -r requirements.txt
python app.py

3.The backend will run at: http://127.0.0.1:5000

4.Access the SmartCart dashboard in a browser: http://127.0.0.1:5000/dashboard

5.Simulate Data Without ESP32
Run the test data script to populate the system manually: python send_test_data.py

6.View Real-Time Results
Dashboard charts update automatically every 10 seconds.
Use filters by customer and date to narrow down analysis.
Export CSV or PDF directly from the dashboard UI.



Step-by-Step Operation

Local Access: Make sure your ESP32 and PC (backend) are on the same local network.

1. Power On
ESP32 boots up, connects to WiFi, initializes LCD, RFID reader, HX711, and sets time via NTP.

2. Scan Product via RFID
User scans RFID tag
ESP32 sends GET request to Flask server /get_product/<uid>
Product info is displayed on LCD

3. Place Item on Scale
If the item is a fruit, weight is measured and multiplied by price
If not, fixed price is used
Item and price are stored in local list and displayed briefly
Green LED flashes to confirm

4. Repeat Scanning for Multiple Items

5. Checkout
Press "Checkout" button
Display total price
Tap payment card (RFID with specific UID)
If valid, sends all cart data via POST to /add_cart_data
Blue LED flashes and LCD shows "Thanks"

6. Reset
Press "Reset" button to clear cart and total


