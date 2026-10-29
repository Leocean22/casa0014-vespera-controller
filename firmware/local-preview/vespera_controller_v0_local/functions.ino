// Function Definitions

// Sensor Fill Light Control
void setSensorLight(bool enabled) {
  digitalWrite(
    SENSOR_LED_PIN,
    enabled ? HIGH : LOW
  );
}

// RGB LED Control 
void showColor(int red, int green, int blue) {
  pixel.setPixelColor(
    0,
    pixel.Color(
      constrain(red, 0, 255),
      constrain(green, 0, 255),
      constrain(blue, 0, 255)
    )
  );

  pixel.show();
}

void showCurrentColor() {
  showColor(R, G, B);
}

void showLockedColor() {
  showColor(lockedR, lockedG, lockedB);
}

// LCD Display 
void printLCDLine(int row, const char* message) {
  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
  lcd.print(message);
}

void updateLCD() {
  char line1[17];
  char line2[17];

  snprintf(
    line1,
    sizeof(line1),
    "R%03d G%03d B%03d",
    R, G, B
  );

  printLCDLine(0, line1);

  switch (state) {
    case MENU:
      if (menuSelection == 0) {
        snprintf(line2, sizeof(line2), ">Sensor  RGB");
      } else {
        snprintf(line2, sizeof(line2), " Sensor >RGB");
      }
      break;

    case SENSOR_PREVIEW:
      snprintf(line2, sizeof(line2), "Sensor: OK?");
      break;

    case RGB_EDIT:
      if (rgbSelection == 0) {
        snprintf(line2, sizeof(line2), "Edit R  Click>");
      } else if (rgbSelection == 1) {
        snprintf(line2, sizeof(line2), "Edit G  Click>");
      } else if (rgbSelection == 2) {
        snprintf(line2, sizeof(line2), "Edit B  Click>");
      } else {
        snprintf(line2, sizeof(line2), "Click to CONFIRM");
      }
      break;

    case COLOR_LOCKED:
      snprintf(line2, sizeof(line2), "Color LOCKED");
      break;
  }

  printLCDLine(1, line2);
}

// Read Sensor Pulse 
unsigned long readPulse(int s2, int s3) {
  digitalWrite(S2_PIN, s2);
  digitalWrite(S3_PIN, s3);

  delayMicroseconds(300);

  return pulseIn(SENSOR_OUT_PIN, LOW, 30000);
}

// Sample Multiple Times and Calculate Average
unsigned long readAveragePulse(int s2, int s3) {
  unsigned long total = 0;
  int validSamples = 0;

  for (int i = 0; i < SAMPLE_COUNT; i++) {
    unsigned long pulse = readPulse(s2, s3);

    if (pulse > 0) {
      total += pulse;
      validSamples++;
    }
  }

  if (validSamples == 0) {
    return 0;
  }

  return total / validSamples;
}

// Update Detected Color
void setDetectedColor(int red, int green, int blue) {
  R = red;
  G = green;
  B = blue;
}

// Print Detection Result 
void printDetectedColor(const char* name) {
  Serial.print("Detected color: ");
  Serial.println(name);
}

// Color Detection 
bool readSensorRGB(int &red, int &green, int &blue) {
  // Read the red, blue, and green channels in sequence
  unsigned long redPulse =
    readAveragePulse(LOW, LOW);

  unsigned long bluePulse =
    readAveragePulse(LOW, HIGH);

  unsigned long greenPulse =
    readAveragePulse(HIGH, HIGH);

  Serial.println("--------------------");

  Serial.print("R pulse = ");
  Serial.println(redPulse);

  Serial.print("G pulse = ");
  Serial.println(greenPulse);

  Serial.print("B pulse = ");
  Serial.println(bluePulse);

  // Set the default output color to black
  red = 0;
  green = 0;
  blue = 0;

  setDetectedColor(0, 0, 0);

  // Check for invalid readings
  if (redPulse == 0 ||
      greenPulse == 0 ||
      bluePulse == 0) {
    printDetectedColor("UNKNOWN - BLACK");
    return true;
  }

  // Treat the color as dark when all three channel signals are weak
  if (redPulse >= DARK_PULSE_THRESHOLD &&
      greenPulse >= DARK_PULSE_THRESHOLD &&
      bluePulse >= DARK_PULSE_THRESHOLD) {
    printDetectedColor("DARK - BLACK");
    return true;
  }

  // A shorter pulse period indicates stronger detected light.
  // Use the reciprocal of each pulse period to estimate relative channel intensity.
  float rValue = 1.0f / redPulse;
  float gValue = 1.0f / greenPulse;
  float bValue = 1.0f / bluePulse;

  float total = rValue + gValue + bValue;

  // Prevent division by zero
  if (total <= 0.0f) {
    printDetectedColor("UNKNOWN - BLACK");
    return true;
  }

  // Convert relative intensity values to the 0-255 range
  red = constrain(
    (int)(rValue / total * 255.0f),
    0, 255
  );

  green = constrain(
    (int)(gValue / total * 255.0f),
    0, 255
  );

  blue = constrain(
    (int)(bValue / total * 255.0f),
    0, 255
  );

  // Update the current color
  setDetectedColor(red, green, blue);

  // Print the RGB values
  Serial.print("RGB = ");
  Serial.print(red);
  Serial.print(", ");
  Serial.print(green);
  Serial.print(", ");
  Serial.println(blue);

  // Identify the dominant color channel
  if (red > green && red > blue) {
    printDetectedColor("RED DOMINANT");
  } else if (green > red && green > blue) {
    printDetectedColor("GREEN DOMINANT");
  } else if (blue > red && blue > green) {
    printDetectedColor("BLUE DOMINANT");
  } else {
    printDetectedColor("MIXED COLOR");
  }

  return true;
}

// Enter the Selected Mode
void enterSelectedMode() {
  if (menuSelection == 0) {
    setSensorLight(true);

    state = SENSOR_PREVIEW;
    lastSensorReadTime = 0;

    Serial.println("Sensor preview mode");
    Serial.println("Sensor light ON");
  } else {
    setSensorLight(false);

    state = RGB_EDIT;
    rgbSelection = 0;

    Serial.println("Manual RGB mode");
    Serial.println("Sensor light OFF");
  }

  updateLCD();
}

// Lock the Current Color 
void lockCurrentColor() {
  lockedR = R;
  lockedG = G;
  lockedB = B;

  state = COLOR_LOCKED;

  setSensorLight(false);

  showLockedColor();
  updateLCD();

  Serial.println("Color locked:");

  Serial.print("R = ");
  Serial.println(lockedR);

  Serial.print("G = ");
  Serial.println(lockedG);

  Serial.print("B = ");
  Serial.println(lockedB);

  Serial.println("Sensor light OFF");
}

// Handle Rotary Encoder Rotation
void handleRotation() {
  int currentCLK = digitalRead(CLK_PIN);

  if (lastCLK == HIGH && currentCLK == LOW) {
    int dt = digitalRead(DT_PIN);

    int step = (dt == HIGH) ? 5 : -5;

    switch (state) {
      case MENU:
        menuSelection += (step > 0) ? 1 : -1;

        if (menuSelection < 0) {
          menuSelection = 1;
        }

        if (menuSelection > 1) {
          menuSelection = 0;
        }

        updateLCD();
        break;

      case RGB_EDIT:
        if (rgbSelection == 0) {
          R = constrain(R + step, 0, 255);
        } else if (rgbSelection == 1) {
          G = constrain(G + step, 0, 255);
        } else if (rgbSelection == 2) {
          B = constrain(B + step, 0, 255);
        }

        showCurrentColor();
        updateLCD();
        break;

      case SENSOR_PREVIEW:
        // Rotation does not change the color in sensor detection mode
        break;

      case COLOR_LOCKED:
        // Rotation does not change the color while it is locked
        break;
    }
  }

  lastCLK = currentCLK;
}

// Handle Rotary Encoder Button 
void handleButton() {
  int reading = digitalRead(SW_PIN);

  // Detect changes in the button reading
  if (reading != lastButtonReading) {
    lastButtonChangeTime = millis();
    lastButtonReading = reading;
  }

  // Debounce the button
  if (millis() - lastButtonChangeTime >= debounceMs) {
    if (reading != stableButtonState) {
      stableButtonState = reading;

      // INPUT_PULLUP: the button reads LOW when pressed
      if (stableButtonState == LOW) {
        Serial.println("Encoder button PRESSED");

        switch (state) {
          case MENU:
            enterSelectedMode();
            break;

          case SENSOR_PREVIEW:
            Serial.println("Confirm sensor color");
            lockCurrentColor();
            break;

          case RGB_EDIT:
            rgbSelection++;

            if (rgbSelection > 3) {
              lockCurrentColor();
            } else {
              updateLCD();
            }
            break;

          case COLOR_LOCKED:
            setSensorLight(false);

            state = MENU;
            updateLCD();

            Serial.println("Back to menu");
            break;
        }
      }
    }
  }
}