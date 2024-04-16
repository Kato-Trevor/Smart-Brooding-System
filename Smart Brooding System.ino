// GSM Module
#include <SoftwareSerial.h>
#define rxPin 10
#define txPin 11

#define DELAY_MS 1000
#define GSM_DELAY_MS 2000
#define RESPONSE_TIMEOUT_MS 2000
#define GPRS_RESPONSE_TIMEOUT_MS 30000
#define DISCONNECT_TIMEOUT_MS 60000

SoftwareSerial SIM800(rxPin, txPin);

const char* APN = "internet";
const char* USER = "";
const char* PASS = "";
const char* THINGSPEAK_HOST = "api.thingspeak.com";
const int THINGSPEAK_PORT = 80;
const char* API_KEY = "2JV0UXXDGYYAX6WL";

// Prototypes
void initializeGSM();
void establishGPRSConnection();
boolean endGPRSConnection();
boolean isGPRSConnected();
boolean waitForExpectedResponse(const char* expectedAnswer = "OK", unsigned int timeout = RESPONSE_TIMEOUT_MS);
void connectToThingSpeak();
void sendDataToThingSpeak(float temperature, float humidity, int gasValue, int lightIntensity, int waterLevel);
void disconnectFromThingSpeak();
void handleThingSpeak(float temperature, float humidity, int gasValue, int lightIntensity, int waterLevel);

// DHT11 Library
#include <dht.h>

// OLED libraries
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for SSD1306 display connected using I2C
#define OLED_RESET -1  // Reset pin
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPin 9      // pin connected to DHT11 sensor
#define MQ7Pin A8     // pin connected to MQ7 gas sensor
#define PRPin A9      // pin connected to photo resistor
#define waterPin A10  // pin connected to water level sensor
#define BuzzerPin 31  // pin connected to the buzzer

#define BLEDPin 8  // pin connected to Blue LED

dht DHT;  // Creates a DHT object

float temperature = 0.0;
float humidity = 0.0;
int gasValue = 0;
int lightIntensity = 0;
int waterLevel = 0;

void setup() {

  Serial.begin(9600);
  pinMode(MQ7Pin, INPUT);
  pinMode(PRPin, INPUT);
  pinMode(BLEDPin, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);

  // initialize the OLED object
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  SIM800.begin(9600);
  Serial.println("Initializing SIM800...");
  initializeGSM();
}

void loop() {
  // Temperature monitoring
  int readData = DHT.read11(DHTPin);

  temperature = DHT.temperature;  // Read temperature
  humidity = DHT.humidity;        // Read humidity

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println("°C");
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println("% ");

  delay(10);

  // Air quality monitoring
  gasValue = analogRead(MQ7Pin);
  Serial.print("Gas value: ");
  Serial.println(gasValue);

  if (gasValue > 300) {
    // Buzzer goes on
    tone(BuzzerPin, 349);
    delay(1000);
  } else {
    // Buzzer goes off
    noTone(BuzzerPin);
    delay(10);
  }

  // Light monitoring
  lightIntensity = analogRead(PRPin);
  Serial.print("Light value:");
  Serial.println(lightIntensity);

  if (lightIntensity < 200) {     // The darker it is the lower the value
    digitalWrite(BLEDPin, HIGH);  //Turn led on
  } else {
    digitalWrite(BLEDPin, LOW);  //Turn led off
  }
  delay(10);

  // Water level - output increases with water
  waterLevel = analogRead(waterPin);
  // as water level increases, resistance reduces (output voltage values increase)

  Serial.print("Water level: ");
  Serial.println(waterLevel);

  delay(10);

  // OLED display
  // Clear the previous content of the display buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Print temperature
  display.setCursor(0, 0);
  display.print(F("Temperature:"));
  display.print(temperature);
  display.print(F(" C"));

  if (temperature >= 30 && temperature <= 33) {
    display.setCursor(0, 10);
    display.print(F("Temp is good"));
  } else {
    display.setCursor(0, 10);
    display.print(F("Temp is poor"));
  }

  // Print humidity
  display.setCursor(0, 20);
  display.print(F("Humidity:"));
  if (humidity >= 40 && humidity <= 60) {
    display.print(F("Good-"));
  } else {
    display.print(F("Poor-"));
  }
  display.print(humidity);
  display.print(F(" %"));

  // Print air quality
  display.setCursor(0, 30);
  display.print(F("Gas Value:"));
  if (gasValue > 400) {
    display.print(F("Poor->"));
  } else {
    display.print(F("Normal->"));
  }
  display.print(gasValue);

  // Print light intensity
  display.setCursor(0, 40);
  display.print(F("Light: "));
  if (lightIntensity < 200) {
    display.print(F("Dark->"));
  } else {
    display.print(F("Normal->"));
  }
  display.print(lightIntensity);

  // Print water Level
  display.setCursor(0, 50);
  display.print(F("Water: "));
  if (waterLevel < 300) {
    display.print(F("Little->"));
  } else {
    display.print(F("Enough->"));
  }
  display.print(waterLevel);
  display.print(F(""));

  // Display the updated content
  display.display();

  // Optional delay to control the refresh rate
  delay(500);

  handleThingSpeak(temperature, humidity, gasValue, lightIntensity, waterLevel);

  delay(50);
}

void initializeGSM() {
  // Send "AT" command to check if the GSM module is responsive
  SIM800.println("AT");
  waitForExpectedResponse();
  delay(GSM_DELAY_MS);
  // Check SIM card status
  SIM800.println("AT+CPIN?");
  waitForExpectedResponse("+CPIN: READY");
  delay(GSM_DELAY_MS);
  // Set GSM module to full functionality (AT+CFUN=1)
  SIM800.println("AT+CFUN=1");
  waitForExpectedResponse();
  delay(GSM_DELAY_MS);
  // Check registration status with the network
  SIM800.println("AT+CREG?");
  waitForExpectedResponse("+CREG: 0,");
  delay(GSM_DELAY_MS);
}

void establishGPRSConnection() {
  // Set the connection type to GPRS
  SIM800.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  waitForExpectedResponse();
  delay(GSM_DELAY_MS);
  // Set the Access Point Name (APN)
  SIM800.println("AT+SAPBR=3,1,\"APN\",\"" + String(APN) + "\"");
  waitForExpectedResponse();
  delay(GSM_DELAY_MS);
  // Set the username (if provided)
  if (USER != "") {
    SIM800.println("AT+SAPBR=3,1,\"USER\",\"" + String(USER) + "\"");
    waitForExpectedResponse();
    delay(GSM_DELAY_MS);
  }
  // Set the password (if provided)
  if (PASS != "") {
    SIM800.println("AT+SAPBR=3,1,\"PWD\",\"" + String(PASS) + "\"");
    waitForExpectedResponse();
    delay(GSM_DELAY_MS);
  }
  // Open the GPRS context
  SIM800.println("AT+SAPBR=1,1");
  waitForExpectedResponse("OK", GPRS_RESPONSE_TIMEOUT_MS);
  delay(GSM_DELAY_MS);
}

boolean isGPRSConnected() {
  SIM800.println("AT+SAPBR=2,1");
  if (waitForExpectedResponse("0.0.0.0") == 1) { return false; }
  return true;
}

boolean endGPRSConnection() {
  SIM800.println("AT+CGATT=0");
  waitForExpectedResponse("OK", DISCONNECT_TIMEOUT_MS);
  return true;
}

boolean waitForExpectedResponse(const char* expectedAnswer, unsigned int timeout) {
  unsigned long previous = millis();
  String response;
  while ((millis() - previous) < timeout) {
    while (SIM800.available()) {
      char c = SIM800.read();
      response.concat(c);
      if (response.indexOf(expectedAnswer) != -1) {
        Serial.println(response);
        return true;
      }
    }
  }
  Serial.println(response);
  return false;
}

void connectToThingSpeak() {
  Serial.println("Connecting to ThingSpeak...");
  SIM800.println("AT+CIPSTART=\"TCP\",\"" + String(THINGSPEAK_HOST) + "\"," + String(THINGSPEAK_PORT));
  if (waitForExpectedResponse("CONNECT OK")) {
    Serial.println("Connected to ThingSpeak");
  } else {
    Serial.println("Connection to ThingSpeak failed");
  }
}

void sendDataToThingSpeak(float temperature, float humidity, int gasValue, int lightIntensity, int waterLevel) {
  String data = "api_key=" + String(API_KEY) + "&field1=" + String(temperature) + "&field2=" + String(humidity) + "&field3=" + String(gasValue) + "&field4=" + String(lightIntensity) + "&field5=" + String(waterLevel);
  String postRequest = "POST /update HTTP/1.1\r\n";
  postRequest += "Host: " + String(THINGSPEAK_HOST) + "\r\n";
  postRequest += "Content-Type: application/x-www-form-urlencoded\r\n";
  postRequest += "Content-Length: " + String(data.length()) + "\r\n\r\n";
  postRequest += data;

  SIM800.println("AT+CIPSEND=" + String(postRequest.length()));
  if (waitForExpectedResponse(">")) {
    SIM800.println(postRequest);
    if (waitForExpectedResponse("OK")) {
      Serial.println("Data sent to ThingSpeak");
    } else {
      Serial.println("Failed to send data to ThingSpeak");
    }
  } else {
    Serial.println("Error in sending data to ThingSpeak");
  }
}

void disconnectFromThingSpeak() {
  SIM800.println("AT+CIPCLOSE");
  waitForExpectedResponse("CLOSE OK");
  Serial.println("Disconnected from ThingSpeak");
  delay(DELAY_MS);
}

void handleThingSpeak(float temperature, float humidity, int gasValue, int lightIntensity, int waterLevel) {
  connectToThingSpeak();
  sendDataToThingSpeak(temperature, humidity, gasValue, lightIntensity, waterLevel);
  disconnectFromThingSpeak();
}