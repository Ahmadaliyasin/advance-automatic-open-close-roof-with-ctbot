#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <CTBot.h>
#include <CTBotDataStructures.h>
#include <CTBotDefines.h>
#include <CTBotInlineKeyboard.h>
#include <CTBotReplyKeyboard.h>
#include <CTBotSecureConnection.h>
#include <CTBotStatusPin.h>
#include <CTBotWifiSetup.h>
#include <Utilities.h>

CTBot myBot;

String ssid = "ssid";     // REPLACE 
String pass = "ssid_pass"; // REPLACE 
String token = "TOKEN";   // REPLACE 
#include <Adafruit_SH110X.h>
#include <splash.h>


#include <Wire.h>
#include <DHT.h>
#include <ESP32Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(4, DHT11); // change your pin DHT
// inialisasi servo
Servo servo1;

const int pinrain = 26;
const int pinlight = 27;
const int buttonUpPin = 14;   // button to move the roof  +10 degrees
const int buttonDownPin = 13; // button to move the roof -10 degrees
const int modeButtonPin = 12; // button to change your mode


int rain, light;
int closeposition = 220;
int openposition = 10;
int mode = 0;            // 0: mode automatic, 1: manual mode
int lastModeButtonState; // to remove and mapping last mode

void setup() {
  Serial.begin(9600);
  Serial.println("Starting Telegram bot...");
  myBot.wifiConnect(ssid, pass);
  myBot.setTelegramToken(token);
  if (myBot.testConnection())
    Serial.println("\nTest success connected");
  else
    Serial.println("\nTest failed connect");
  dht.begin();
  servo1.attach(25);
  pinMode(pinrain, INPUT);
  pinMode(pinlight, INPUT);
  pinMode(buttonUpPin, INPUT_PULLUP);
  pinMode(buttonDownPin, INPUT_PULLUP);
  pinMode(modeButtonPin, INPUT_PULLUP);

  lastModeButtonState = digitalRead(modeButtonPin);
}

void loop() {
 TBMessage msg;
  // Handle incoming Telegram messages
  if (myBot.getNewMessage(msg)) {
    // Check for specific commands
    if (msg.text.equalsIgnoreCase("manual mode")) {
      mode = 1;
      myBot.sendMessage(msg.sender.id, "manual mode active");
    } else if (msg.text.equalsIgnoreCase("automatic mode")) {
      mode = 0;
      myBot.sendMessage(msg.sender.id, "automatic mode active");
    } else if (msg.text.equalsIgnoreCase("close roof")) {
      servo1.write(closeposition);
      myBot.sendMessage(msg.sender.id, "roof has been closed");
    } else if (msg.text.equalsIgnoreCase("open roof")) {
      servo1.write(openposition);
      myBot.sendMessage(msg.sender.id, "roof has been opened");
    } else if (msg.text.equalsIgnoreCase("status")) {
      // Display all status information
      float humidity = dht.readHumidity();
      float temperature = dht.readTemperature();

      String statusMessage = "Mode: " + String(mode == 0 ? "automatic" : "Manual") + "\n";
      statusMessage += "rain: " + String(rain == HIGH ? "Yes" : "No") + "\n";
      statusMessage += "light: " + String(light) + " (" + (light < 500 ? "dark" : "bright") + ")\n";
      statusMessage += "humiity: " + String(humidity) + "%, temperature: " + String(temperature) + "°C\n";
      statusMessage += "Position Servo: " + String(map(servo1.read(), 10, 270, 0, 100)) + "% dari 10-270 derajat";

      myBot.sendMessage(msg.sender.id, statusMessage);
    }
  }

  rain = digitalRead(pinrain);
  light = analogRead(pinlight);
  

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("failed check sensor DHT11!");
    return;
  }

  int modeButtonState = digitalRead(modeButtonPin);

  // Tombol untuk mengubah mode (manual atau otomatis)
  if (modeButtonState == LOW && lastModeButtonState == HIGH) {
    mode = 1 - mode; // Toggle mode (0 -> 1, 1 -> 0)
    delay(200);      // Delay untuk menghindari bouncing tombol
  }

  // Tombol untuk menggerakkan servo +10 derajat
  if (digitalRead(buttonUpPin) == LOW && mode == 1) {
    servo1.write(servo1.read() + 10);
    delay(300); // Delay untuk menghindari bouncing tombol
  }

  // Tombol untuk menggerakkan servo -10 derajat
 if (digitalRead(buttonDownPin) == LOW && mode == 1) {
    servo1.write(servo1.read() - 10);
    delay(300); // Delay untuk menghindari bouncing tombol
}

  Serial.print("Mode: ");
  Serial.println(mode == 0 ? "Otomatis" : "Manual");
  
  Serial.print("rain: ");
  Serial.print(rain);
  Serial.print(", light: ");
  Serial.println(light);
  Serial.println();
  Serial.print("humidity: ");
  Serial.print(humidity);
  Serial.print("%, Suhu: ");
  Serial.print(temperature);
  Serial.println("°C");

  // Mode otomatis
 if (mode == 0) {
    if (rain == 1 && light <= 500) {
        servo1.write(closeposition);
    } else if (rain == 0 && light >= 500) {
        servo1.write(openposition);
    } else {
        if (temperature > 25 || humidity > 80) {
            servo1.write(openposition);
        } else {
            servo1.write(closeposition);
        }
    }
  }

 

  

       
 display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("Mode: ");
  display.println(mode == 0 ? "automatic" : "Manual");

  display.print("rain: ");
  display.print(rain);
  display.print(", light: ");
  display.print(light);
  display.print(" (");
  display.print(light < 500 ? "dark" : "bright");
  display.println(")");

  display.print("humidity: ");
  display.print(humidity);
  display.print("%, temperature: ");
  display.println(temperature);

  display.print("position Servo: ");
  display.print(map(servo1.read(), 10, 270, 0, 100));
  display.println("% dari 10-270 degrees");

  display.display();
  lastModeButtonState = modeButtonState;

  delay(300);
}