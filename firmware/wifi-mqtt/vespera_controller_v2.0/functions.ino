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

// Color Detection -> Color classification
// Supports calibrated YELLOW, RED, GREEN and BLUE.
// Unrecognized colors output RGB(0,0,0).
// ==================================================

bool readSensorRGB(int &red, int &green, int &blue) {
  unsigned long redPulse =
    readAveragePulse(LOW, LOW);

  unsigned long bluePulse =
    readAveragePulse(LOW, HIGH);

  unsigned long greenPulse =
    readAveragePulse(HIGH, HIGH);

  Serial.println("--------------------");

  Serial.print("R pulse = ");
  Serial.print(redPulse);
  Serial.println(" us");

  Serial.print("G pulse = ");
  Serial.print(greenPulse);
  Serial.println(" us");

  Serial.print("B pulse = ");
  Serial.print(bluePulse);
  Serial.println(" us");

  // Default to black if the color cannot be identified.
  red = 0;
  green = 0;
  blue = 0;
  setDetectedColor(0, 0, 0);

  // Reject invalid sensor readings.
  if (redPulse == 0 ||
      greenPulse == 0 ||
      bluePulse == 0) {
    printDetectedColor("UNKNOWN - BLACK");
    return true;
  }

  // Reject very dark readings.
  if (redPulse >= DARK_PULSE_THRESHOLD &&
      greenPulse >= DARK_PULSE_THRESHOLD &&
      bluePulse >= DARK_PULSE_THRESHOLD) {
    printDetectedColor("DARK / NO VALID COLOR");
    return true;
  }

  bool isYellow = false;
  bool isRed = false;
  bool isGreen = false;
  bool isBlue = false;

  // --------------------------------------------------
  // A. Red
  // --------------------------------------------------

  if (redPulse >= 10 &&
      redPulse <= 80 &&
      greenPulse >= redPulse * 1.6f &&
      bluePulse >= redPulse * 1.35f) {
    isRed = true;
  }

  if (redPulse >= 131 &&
      redPulse <= 158 &&
      greenPulse >= 332 &&
      greenPulse <= 440 &&
      bluePulse >= 279 &&
      bluePulse <= 346) {
    isRed = true;
  }

  // --------------------------------------------------
  // B. Green
  // --------------------------------------------------

  if (greenPulse >= 50 &&
      greenPulse <= 125 &&
      redPulse >= greenPulse * 1.03f &&
      bluePulse >= greenPulse * 1.08f) {
    isGreen = true;
  }

  if (redPulse >= 100 &&
      redPulse <= 160 &&
      greenPulse >= 150 &&
      greenPulse <= 205 &&
      bluePulse >= 160 &&
      bluePulse <= 220) {
    isGreen = true;
  }

  if (redPulse >= 225 &&
      redPulse <= 277 &&
      greenPulse >= 328 &&
      greenPulse <= 380 &&
      bluePulse >= 367 &&
      bluePulse <= 408) {
    isGreen = true;
  }

  // --------------------------------------------------
  // C. Blue
  // --------------------------------------------------

  if (bluePulse >= 10 &&
      bluePulse <= 75 &&
      redPulse >= bluePulse * 1.5f &&
      greenPulse >= bluePulse * 1.25f) {
    isBlue = true;
  }

  if (redPulse >= 252 &&
      redPulse <= 289 &&
      greenPulse >= 242 &&
      greenPulse <= 256 &&
      bluePulse >= 165 &&
      bluePulse <= 174) {
    isBlue = true;
  }

  // --------------------------------------------------
  // D. Yellow
  // --------------------------------------------------

  if (redPulse >= 10 &&
      redPulse <= 110 &&
      greenPulse >= 10 &&
      greenPulse <= 110 &&
      bluePulse >= 55 &&
      bluePulse >= redPulse * 1.20f &&
      bluePulse >= greenPulse * 1.15f &&
      redPulse <= greenPulse * 1.80f &&
      greenPulse <= redPulse * 1.80f) {
    isYellow = true;
  }

  if (redPulse >= 76 &&
      redPulse <= 85 &&
      greenPulse >= 125 &&
      greenPulse <= 142 &&
      bluePulse >= 175 &&
      bluePulse <= 195) {
    isYellow = true;
  }

  // --------------------------------------------------
  // E. Output the detected color.
  // If no color or multiple colors match, output black.
  // --------------------------------------------------

  int matchedColors = 0;

  if (isYellow) matchedColors++;
  if (isRed) matchedColors++;
  if (isGreen) matchedColors++;
  if (isBlue) matchedColors++;

  if (matchedColors != 1) {
    setDetectedColor(0, 0, 0);
    printDetectedColor("UNKNOWN - BLACK");
  } else if (isYellow) {
    setDetectedColor(255, 255, 0);
    printDetectedColor("PURE YELLOW");
  } else if (isRed) {
    setDetectedColor(255, 0, 0);
    printDetectedColor("PURE RED");
  } else if (isGreen) {
    setDetectedColor(0, 255, 0);
    printDetectedColor("PURE GREEN");
  } else {
    setDetectedColor(0, 0, 255);
    printDetectedColor("PURE BLUE");
  }

  red = R;
  green = G;
  blue = B;

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

  // Request MQTT publishing of the confirmed color
  pendingColorPublish = true;
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
