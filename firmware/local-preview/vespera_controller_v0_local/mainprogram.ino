// Main Program

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

  // Update the initial display
  updateLCD();

  Serial.println("RGB and TCS3200 controller ready");
  Serial.println("Basic RGB recognition enabled");
  Serial.println("WS2812 initialized OFF");
  Serial.println("Sensor light OFF at startup");
}

// Main Loop
void loop() {
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