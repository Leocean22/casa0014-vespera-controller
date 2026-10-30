// Main Program

// Prepare the MQTT client and display the device information
void prepareMqttClient() {
  // Initialize the onboard RGB status LED on the MKR1010
  WiFiDrv::pinMode(25, OUTPUT); // R
  WiFiDrv::pinMode(26, OUTPUT); // G
  WiFiDrv::pinMode(27, OUTPUT); // B

  LedRed();

  // Generate a unique client ID using the device MAC address
  byte mac[6];
  WiFi.macAddress(mac);

  char macStr[18];

  snprintf(
    macStr,
    sizeof(macStr),
    "%02X%02X%02X%02X%02X%02X",
    mac[5],
    mac[4],
    mac[3],
    mac[2],
    mac[1],
    mac[0]
  );

  clientId = "MKR1010_Sender_" + String(macStr);

  Serial.print(F("Device Client ID: "));
  Serial.println(clientId);

  Serial.print(F("Publishing to topic: "));
  Serial.println(mqtt_topic);

  // Configure the MQTT broker
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(512);
  mqttClient.setKeepAlive(30);
}

// Check WiFi and MQTT connections and publish a confirmed color
void serviceNetwork() {
  // Reconnect to WiFi when disconnected
  if (WiFi.status() != WL_CONNECTED) {
    startWifi();
  }

  // Reconnect to MQTT when WiFi is available
  if (WiFi.status() == WL_CONNECTED &&
      !mqttClient.connected()) {
    reconnectMQTT();
  }

  // Keep the MQTT connection alive
  mqttClient.loop();

  // Publish the confirmed RGB color when connected
  if (pendingColorPublish &&
      mqttClient.connected()) {

    send_colour_to_all(
      lockedR,
      lockedG,
      lockedB
    );

    pendingColorPublish = false;

    Serial.println(F("Confirmed color published to Vespera."));
  }
}

// System Initialization
void setup() {
  Serial.begin(115200);

  // Initialize the TCS3200 color sensor
  pinMode(S0_PIN, OUTPUT);
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(S3_PIN, OUTPUT);
  pinMode(SENSOR_OUT_PIN, INPUT);

  // Set the sensor output frequency scaling to 20%
  digitalWrite(S0_PIN, HIGH);
  digitalWrite(S1_PIN, LOW);

  // Initialize the sensor fill light and keep it OFF at startup
  pinMode(SENSOR_LED_PIN, OUTPUT);
  setSensorLight(false);

  // Initialize the rotary encoder
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);

  lastCLK = digitalRead(CLK_PIN);
  lastButtonReading = digitalRead(SW_PIN);
  stableButtonState = lastButtonReading;

  // Initialize the LCD
  Wire.begin();
  lcd.init();
  lcd.setRGB(255, 255, 255);

  // Initialize the WS2812 RGB LED
  pixel.begin();
  pixel.setBrightness(50);
  pixel.clear();
  pixel.show();

  // Prepare the WiFi and MQTT client
  prepareMqttClient();

  // Connect to WiFi
  startWifi();

  // Update the initial display
  updateLCD();

  Serial.println("RGB and TCS3200 controller ready");
  Serial.println("Basic RGB recognition enabled");
  Serial.println("WS2812 initialized OFF");
  Serial.println("Sensor light OFF at startup");
}

// Main Loop
void loop() {
  // Check WiFi and MQTT connections
  serviceNetwork();

  // Continuously check the rotary encoder and its button
  handleRotation();
  handleButton();

  // Read the color only in sensor detection mode
  if (state == SENSOR_PREVIEW) {
    if (millis() - lastSensorReadTime >= sensorIntervalMs) {
      lastSensorReadTime = millis();

      int sensorR = 0;
      int sensorG = 0;
      int sensorB = 0;

      if (readSensorRGB(sensorR, sensorG, sensorB)) {
        R = sensorR;
        G = sensorG;
        B = sensorB;

        showCurrentColor();
        updateLCD();
      }
    }
  }
}

// Sets all 72 Vespera LEDs to the given RGB color and publishes it to MQTT
void send_colour_to_all(int r, int g, int b) {
  if (mqttClient.connected()) {
    for (int led = 0; led < num_leds; led++) {
      RGBpayload[led * 3 + 0] =
        (byte)constrain(r, 0, 255);

      RGBpayload[led * 3 + 1] =
        (byte)constrain(g, 0, 255);

      RGBpayload[led * 3 + 2] =
        (byte)constrain(b, 0, 255);
    }

    boolean published = mqttClient.publish(
      mqtt_topic.c_str(),
      RGBpayload,
      payload_size
    );

    if (published) {
      Serial.print(F("Published RGB: "));
      Serial.print(r);
      Serial.print(F(", "));
      Serial.print(g);
      Serial.print(F(", "));
      Serial.println(b);
    } else {
      Serial.println(F("MQTT publish failed."));
    }
  } else {
    Serial.println(F("MQTT client not connected, cannot publish."));
  }
}

// Turns all 72 Vespera LEDs off
void send_all_off() {
  send_colour_to_all(0, 0, 0);
}