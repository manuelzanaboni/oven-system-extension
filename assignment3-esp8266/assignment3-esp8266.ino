// After talking to Davide Marelli, it seems that's not possible to dim LCD backlight because of the I2C board.
// So i decided to simulate this behavior by using PWM on pin D8 and wiring it to a standard led.
// The wired led lights up accordingly with the level of ambient lighting. 

#include <ESP8266WiFi.h>          // WiFi library
#include <MQTT.h>                 // MQTT
#include <LiquidCrystal_I2C.h>    // display library
#include <Wire.h>                 // I2C library
#include <ArduinoJson.h>          // JSON support
#include <ESP8266TelegramBOT.h>
#include "secrets.h"              // Wifi - MySQL - MQTT credential
#include "config.h"               // Configurations

#define DISPLAY_CHARS 16          // number of characters on a line
#define DISPLAY_LINES 2           // number of display lines
#define DISPLAY_ADDR 0x27         // display address on I2C bus

#define PHOTORESISTOR A0

#define LED_WIFI_LOW D0
#define LED_BURNER D3
#define LED_RESISTANCE D4

#define BUTTON1 D5 
#define BUTTON2 D6 
#define BUTTON3 D7 
#define BUTTON_DEBOUNCE_DELAY 20      // buttons debounce

#define PWM_PIN D8

#define RSSI_THRESHOLD -60            // WiFi signal strength threshold

// Display config
LiquidCrystal_I2C lcd(DISPLAY_ADDR, DISPLAY_CHARS, DISPLAY_LINES);
// custom degree ° character
byte degreeChar[] = {
  B00100,
  B01010,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000
};

// custom lightning character
byte lightningChar[] = {
  B00110,
  B01100,
  B11000,
  B11111,
  B00011,
  B00110,
  B01100,
  B11000
};

// custom fire character
byte fireChar[] = {
  B01000,
  B00100,
  B00110,
  B01100,
  B11110,
  B10011,
  B10001,
  B01110
};

// WiFi config
char ssid[] = SECRET_SSID;   // network SSID
char pass[] = SECRET_PASS;   // network password
#ifdef IP
IPAddress ip(IP);
IPAddress subnet(SUBNET);
IPAddress dns(DNS);
IPAddress gateway(GATEWAY);
#endif
WiFiClient client;

// MQTT data
MQTTClient mqttClient(256);         // handles the MQTT communication protocol

struct data_type {
  bool valid;
  int id;
  float oven_temp;
  float floor_temp;
  float puffer_temp;
  float fumes_temp;
  int delta_press_oven;
  int delta_press_gas;
  int set_point;
  int burner_status;
  int resistance_status;
  int wifi_signal;
  String timestamp;
  String to_string;
} last_data;

int visual = 1;       // what visualization to display

// Telegram Bot
TelegramBOT bot(BOT_TOKEN, BOT_NAME, BOT_USERNAME);
#define BOT_MTBS 2000   // mean time between scans for new messages
long botLastScanTime;   // last time messages' scan has been done

void setup() {
  WiFi.mode(WIFI_STA);

  analogWrite(PWM_PIN, 200);
  analogWriteRange(255);

  Serial.begin(115200);
  // wait for Serial initialization
  delay(2000);

  // LCD setup
  Serial.println(F("\nCheck LCD connection..."));
  Wire.begin();
  Wire.beginTransmission(DISPLAY_ADDR);
  byte error = Wire.endTransmission();
  
  if (error == 0) {
    Serial.println(F("LCD found."));
    lcd.begin(DISPLAY_CHARS, DISPLAY_LINES);   // initialize the lcd
    lcd.createChar(0, degreeChar);             // define a custom character
    lcd.createChar(1, lightningChar);          // define a custom character
    lcd.createChar(2, fireChar);               // define a custom character
  } else {
    Serial.print(F("LCD not found. Error "));
    Serial.println(error);
    Serial.println(F("Check connections and configuration. Reset to try again!"));
    while (true)
      delay(1); 
  }

  // setup MQTT
  mqttClient.begin(MQTT_BROKERIP, 1883, client);        // setup communication with MQTT broker
  mqttClient.onMessage(mqtt_message_received);            // callback on message received from MQTT broker

  // LEDS
  pinMode(LED_WIFI_LOW, OUTPUT);
  pinMode(LED_BURNER, OUTPUT);
  pinMode(LED_RESISTANCE, OUTPUT);
  
  digitalWrite(LED_WIFI_LOW, HIGH);
  digitalWrite(LED_BURNER, HIGH);
  digitalWrite(LED_RESISTANCE, HIGH);

  // buttons
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  
  Serial.println(F("\n\nSetup completed.\n\n"));
}

void loop() {
  static int boot = 0;
  
  static byte led_wifi_low_state = HIGH;
  static byte led_burner_state = HIGH;
  static byte led_resistance_state = HIGH;

  static unsigned long photoresistor_time = 0;
  static unsigned long test_time = 0;

  if (boot == 0) {
    lcd.setBacklight(255);    // set backlight to maximum
    lcd.home();               // move cursor to 0,0
    lcd.clear();              // clear text
    lcd.print("Hello LCD");   // show text
    delay(2000);
    lcd.setCursor(0, 0);
    lcd.print("Waiting for data");
  }
  boot = 1;

//  Serial.println("Waking up from Modem Sleep");
//  WiFi.forceSleepWake(); ////////////////////////////////////
//  delay(1);
  
   // connect to WiFi (if not already connected)
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("Connecting to SSID: "));
    Serial.println(SECRET_SSID);

#ifdef IP
    WiFi.config(ip, dns, gateway, subnet);   // by default network is configured using DHCP
#endif

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(F("."));
      delay(250);
    }
    Serial.println(F("\nConnected!"));
    printWifiStatus();

    bot.begin();   // start the Telegram bot
  }

  mqttClient.loop();       // MQTT client loop
  connect_to_MQTT_broker();   // connect to MQTT broker (if not already connected)
  
  // manage LEDS - calls to leds service (leds tab)
  led_wifi_low_state = manage_wifi_led(led_wifi_low_state);
  led_burner_state = manage_burner_led(led_burner_state);
  led_resistance_state = manage_resistance_led(led_resistance_state);

  if(millis() - photoresistor_time > 1000) {
    check_photoresistor();
    photoresistor_time = millis();
  }
  
  if (millis() > botLastScanTime + BOT_MTBS) {
    bot.getUpdates(bot.message[0][1]);   // launch API GetUpdates up to xxx message
    botExecMessages();                   // process received messages
    botLastScanTime = millis();
  }
  

  // buttons
  if (isButton1Pressed()) {     // BUTTON1 pressed
    Serial.println(F("BUTTON1 pressed"));
    visual = 1;
    update_display();
  }

    if (isButton2Pressed()) {   // BUTTON2 pressed
    Serial.println(F("BUTTON2 pressed"));
    visual = 2;
    update_display();
  }
  
    if (isButton3Pressed()) {   // BUTTON3 pressed
    Serial.println(F("BUTTON3 pressed"));
    visual = 3;
    update_display();
  }

//  WiFi.mode( WIFI_OFF );
//  WiFi.forceSleepBegin();
//  Serial.println("Entering Modem Sleep");
}


void check_photoresistor() {
  unsigned int lightSensorValue = lightSensorValue = analogRead(PHOTORESISTOR);
//  Serial.print(F("Light sensor value: "));
//  Serial.println(lightSensorValue);
  
  unsigned int mappedValue = map(lightSensorValue, 0, 1023, 0, 255);    
  analogWrite(PWM_PIN, mappedValue);
}

void update_display() {
  if(last_data.valid) {
    lcd.clear();
    lcd.setCursor(0, 0);
    switch(visual) {
      case 1:
        lcd.print("T1:");
        lcd.print((int)last_data.oven_temp);
        lcd.write(0);                           // degree custom char
        lcd.print("C T2:");
        lcd.print((int)last_data.floor_temp);
        lcd.write(0);
        lcd.print("C");
        lcd.setCursor(0, 1);
        lcd.print("T3:");
        lcd.print((int)last_data.puffer_temp);
        lcd.write(0);
        lcd.print("C T4:");
        lcd.print((int)last_data.fumes_temp);
        lcd.write(0);
        lcd.print("C");
      break;
 
      case 2:
       lcd.print("Press1: ");
       lcd.print(last_data.delta_press_oven);
       lcd.print(" Pa");
       lcd.setCursor(0, 1);
       lcd.print("Press2: ");
       lcd.print(last_data.delta_press_gas);  
       lcd.print(" Pa");
      break;

      case 3:
        lcd.write(2);
        if(last_data.burner_status == 1)
          lcd.print(": ON - ");
        else
          lcd.print(": OFF - ");
          
        lcd.write(1);
        if(last_data.resistance_status == 1)
          lcd.print(": ON");
        else
          lcd.print(": OFF");
          
        lcd.setCursor(0, 1);
        lcd.print("SetPoint: ");
        lcd.print(last_data.set_point);
        lcd.print(" ");
        lcd.write(0);
        lcd.print("C");
      break;
      
      default:
      break;
    }
  }
}

void printWifiStatus() {
  Serial.println(F("\n=== WiFi connection status ==="));

  // SSID
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // signal strength
  Serial.print(F("Signal strength (RSSI): "));
  Serial.print(WiFi.RSSI());
  Serial.println(F(" dBm"));

  // current IP
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());

  // subnet mask
  Serial.print(F("Subnet mask: "));
  Serial.println(WiFi.subnetMask());

  // gateway
  Serial.print(F("Gateway IP: "));
  Serial.println(WiFi.gatewayIP());

  // DNS
  Serial.print(F("DNS IP: "));
  Serial.println(WiFi.dnsIP());

  Serial.println(F("==============================\n"));
}
