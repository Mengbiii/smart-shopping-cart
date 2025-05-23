// Smart Cart ESP32 Main Code with WiFi Integration
#include <Wire.h>
#include <LiquidCrystal.h>
#include <MFRC522_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HX711.h>
#include <ArduinoJson.h>
#include <vector>
#include <ctime>  // For timestamp
#include <time.h>

// === LCD Configuration ===
LiquidCrystal lcd(4, 2, 25, 26, 18, 17);

// === RFID Configuration ===
#define RST_PIN 15
MFRC522_I2C mfrc522(0x2C, RST_PIN);

// === HX711 Configuration ===
#define LOADCELL_DOUT_PIN 39
#define LOADCELL_SCK_PIN 16
HX711 scale;

// === Button Configuration ===
#define BUTTON_CHECKOUT 19
#define BUTTON_RESET    23

// === LED Configuration ===
#define GREEN_LED_PIN  13
#define BLUE_LED_PIN   12

// === WiFi and Backend ===
const char* ssid = "yangYang";
const char* password = "Yukikaze";
const char* serverUrl = "http://192.168.137.1:5000/get_product/";
const char* postUrl = "http://192.168.137.1:5000/add_cart_data";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;  // UTC+8
const int daylightOffset_sec = 0;

float totalPrice = 0.0;
String lastUID = "";
bool checkoutRequested = false;

struct ProductInfo {
  String name;
  float price;
  bool isFruit;
  bool success;
};

struct CartItem {
  String name;
  float price;
  float weight;
};
std::vector<CartItem> cartItems;

//Fetch product details from backend by UID via HTTP GET.
//Returns product name, price, type (fruit or not), and success status.
ProductInfo fetchProductInfo(String uid) {
  ProductInfo info;
  info.success = false;

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String fullUrl = serverUrl + uid;
    Serial.println("Requesting: " + fullUrl);
    http.begin(fullUrl);

    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString();

      StaticJsonDocument<200> doc;
      DeserializationError error = deserializeJson(doc, payload);
      if (!error) {
        info.name = doc["name"] | "Unknown";
        info.price = doc["price"] | 0.0;
        info.isFruit = doc["is_fruit"] | false;
        info.success = true;
      } else {
        Serial.println("JSON parse failed: " + String(error.c_str()));
      }
    } else {
      Serial.println("HTTP GET failed, code: " + String(httpCode));
    }
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }

  return info;
}

//Sends the current cart data (items, prices, weights) to backend via HTTP POST.
//Includes timestamp and clears the cart after sending.
void sendReceipt(String customer) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected, cannot send receipt.");
    return;
  }

  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char timeBuffer[30];
  strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%dT%H:%M:%S", timeinfo);
  String timestamp(timeBuffer);

  String payload = "{\"customer\":\"" + customer + "\",\"items\":[";
  for (size_t i = 0; i < cartItems.size(); i++) {
    payload += "{\"name\":\"" + cartItems[i].name + "\",";
    payload += "\"price\":" + String(cartItems[i].price, 2) + ",";
    payload += "\"weight\":" + String(cartItems[i].weight, 3) + "}";
    if (i < cartItems.size() - 1) payload += ",";
  }
  payload += "],\"total\":" + String(totalPrice, 2);
  payload += ",\"timestamp\":\"" + timestamp + "\"";
  payload += "}";

  HTTPClient http;
  http.begin(postUrl);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(payload);
  Serial.println("POST status: " + String(code));
  Serial.println("Payload: " + payload);
  http.end();

  cartItems.clear(); 
}

//Initializes WiFi, LCD, RFID reader, load cell, and NTP time sync.
//Prepares all hardware and displays “Scan Goods” on LCD.
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_CHECKOUT, INPUT_PULLUP);
  pinMode(BUTTON_RESET, INPUT_PULLUP);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, LOW);

  lcd.begin(16, 2);
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected. IP:");
  Serial.println(WiFi.localIP());

  lcd.clear(); lcd.print("WiFi Connected"); delay(1000);
  Wire.begin(21, 22);
  mfrc522.PCD_Init();

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.tare();
  scale.set_scale(-295400.6266);

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 100000) delay(500);

  lcd.clear(); lcd.print("Scan Goods:");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Serial.println("Waiting for time...");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    delay(1000);
  }

  Serial.println("Time synchronized.");
}

//Main loop: checks for reset or checkout button input.
//If not checking out, listens for RFID scan to add products.
void loop() {
  if (digitalRead(BUTTON_RESET) == LOW) {
    totalPrice = 0.0;
    cartItems.clear(); 
    lcd.clear(); 
    lcd.setCursor(0, 0);
    lcd.print("Reset!");
    lcd.setCursor(0, 1);
    lcd.print("Scan Goods:");
    digitalWrite(BLUE_LED_PIN, LOW);
    delay(500);
  }
  if (!checkoutRequested) {
    checkRFID();
    if (digitalRead(BUTTON_CHECKOUT) == LOW) {
      checkoutRequested = true;
      checkOut();
    }
  }
}

//Handles RFID tag reading, fetches product info, waits for item placement.
//Calculates price (weighted if fruit), updates LCD and adds item to cart.
void checkRFID() {
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return;
  String uidStr = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) uidStr += "0";
    uidStr += String(mfrc522.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  ProductInfo info = fetchProductInfo(uidStr);
  if (!info.success) {
    lcd.clear(); lcd.print("Lookup Failed"); delay(1500); lcd.clear(); lcd.print("Scan Goods:"); return;
  }

  lcd.clear(); lcd.print(info.name); lcd.setCursor(0, 1); lcd.print("Place item...");
  float base = scale.get_units(10);
  bool placed = false;
  unsigned long start = millis();
  while (millis() - start < 8000) {
    if (abs(scale.get_units(10) - base) > 0.05) { placed = true; break; }
    delay(300);
  }
  if (!placed) {
    lcd.clear(); lcd.print("Not Confirmed"); delay(1500); lcd.clear(); lcd.print("Scan Goods:"); return;
  }

  float weight = 0.0;
  float price = 0.0;
  if (info.isFruit) {
    weight = scale.get_units(10);
    price = weight * info.price;

    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(info.name);
    lcd.setCursor(0, 1); lcd.print("Weight: " + String(weight, 2) + "kg");
    delay(2000);
  } else {
    price = info.price;
  }


  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(info.name);
  lcd.setCursor(0, 1); lcd.print("Added: $" + String(price, 2));
  digitalWrite(GREEN_LED_PIN, HIGH); delay(300); digitalWrite(GREEN_LED_PIN, LOW);
  delay(1500); lcd.clear(); lcd.print("Scan Goods:");

  CartItem item = {info.name, price, weight};
  cartItems.push_back(item);
  totalPrice += price;
}

//Shows total price and waits for authorized RFID card for payment.
//If confirmed, sends cart to server and resets cart state.
void checkOut() {
  lcd.clear(); lcd.setCursor(0, 0); lcd.print("Total: $" + String(totalPrice, 2));
  lcd.setCursor(0, 1); lcd.print("Tap card to pay");
  unsigned long startTime = millis();
  while (millis() - startTime < 20000) {
    if (digitalRead(BUTTON_RESET) == LOW) {
      lcd.clear(); lcd.print("Cancelled"); delay(1500);
      lcd.clear(); lcd.print("Scan Goods:"); checkoutRequested = false; return;
    }
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      String uidStr = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) uidStr += "0";
        uidStr += String(mfrc522.uid.uidByte[i], HEX);
      }
      uidStr.toUpperCase();
      if (uidStr == "0418BBFA3F7481") {
        lcd.clear(); lcd.print("Confirmed"); lcd.setCursor(0, 1); lcd.print("Sending...");
        sendReceipt("Customer1");
        digitalWrite(BLUE_LED_PIN, HIGH); delay(1500); digitalWrite(BLUE_LED_PIN, LOW);
        totalPrice = 0.0; 
        lcd.clear(); lcd.print("THANKS!"); delay(2000);
        lcd.clear(); lcd.print("Scan Goods:"); checkoutRequested = false; return;
      } else {
        lcd.clear(); lcd.print("Invalid Card"); delay(1000);
        lcd.clear(); lcd.setCursor(0, 0); lcd.print("Total: $" + String(totalPrice, 2));
        lcd.setCursor(0, 1); lcd.print("Tap card to pay");
      }
    }
  }
  lcd.clear(); lcd.print("Timeout"); delay(2000);
  lcd.clear(); lcd.print("Scan Goods:"); checkoutRequested = false;
}
