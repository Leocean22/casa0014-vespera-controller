#include <Wire.h>
#include <DFRobot_RGBLCD1602.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"
#include <utility/wifi_drv.h>

// Hardware Setup and Global Variables

// TCS3200 Color Sensor
const int S0_PIN = 2;
const int S1_PIN = 3;
const int S2_PIN = 4;
const int S3_PIN = 5;
const int SENSOR_OUT_PIN = 6;

// Rotary Encoder
const int CLK_PIN = 7;
const int DT_PIN = 8;
const int SW_PIN = 9;

// WS2812 RGB LED
const int LED_PIN = 10;
const int LED_COUNT = 1;

// Sensor Fill Light
const int SENSOR_LED_PIN = 13;

// LCD Display
DFRobot_RGBLCD1602 lcd(0x60, 16, 2);

// WS2812 LED Object
Adafruit_NeoPixel pixel(
  LED_COUNT,
  LED_PIN,
  NEO_GRB + NEO_KHZ800
);

// Application States
enum AppState {
  MENU,
  SENSOR_PREVIEW,
  RGB_EDIT,
  COLOR_LOCKED
};

AppState state = MENU;

// Menu selection: 0 = Sensor Detection, 1 = Manual RGB
int menuSelection = 0;

// RGB adjustment selection: 0 = Red, 1 = Green, 2 = Blue, 3 = Confirm
int rgbSelection = 0;

// Current RGB color values
int R = 0;
int G = 0;
int B = 0;

// Locked RGB color values
int lockedR = 0;
int lockedG = 0;
int lockedB = 0;

// Rotary Encoder State
int lastCLK = HIGH;
int lastButtonReading = HIGH;
int stableButtonState = HIGH;

unsigned long lastButtonChangeTime = 0;
const unsigned long debounceMs = 35;

// Color Detection Parameters
const int SAMPLE_COUNT = 3;

const unsigned long sensorIntervalMs = 500;
unsigned long lastSensorReadTime = 0;

// Treat the color as dark when all three channel signals are weak
const unsigned long DARK_PULSE_THRESHOLD = 300;

// WiFi and MQTT Configuration
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;
const char* ssid1 = SECRET_SSID1;
const char* password1 = SECRET_PASS1;

const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;

const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;

// Create WiFi and MQTT client objects
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Update this with your allocated Vespera light ID
String lightId = "26";

// MQTT topic for the allocated Vespera light
String mqtt_topic = "student/CASA0014/luminaire/" + lightId;

// Unique MQTT client ID based on the device MAC address
String clientId = "";

// Vespera light configuration
const int num_leds = 72;
const int payload_size = num_leds * 3;

byte RGBpayload[payload_size];

// Set to true when a confirmed color needs to be published
bool pendingColorPublish = false;

// MQTT function declarations
void prepareMqttClient();
void serviceNetwork();