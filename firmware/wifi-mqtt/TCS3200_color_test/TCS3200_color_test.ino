// TCS3200 Colour Sensor Test
// Reads red, green, and blue pulse widths.
// The sensor illumination LED is turned on during testing.
// Keep the sensor distance and lighting conditions consistent.

const int S0_PIN = 2;
const int S1_PIN = 3;
const int S2_PIN = 4;
const int S3_PIN = 5;
const int SENSOR_OUT_PIN = 6;

// Use the same illumination LED pin as the main project.
const int SENSOR_LED_PIN = 13;

// Number of readings used to calculate each average.
const int SAMPLE_COUNT = 3;


// Read the pulse width for the selected colour filter.
// The S2 and S3 pins select the colour channel.
unsigned long readPulse(int s2, int s3) {
  digitalWrite(S2_PIN, s2);
  digitalWrite(S3_PIN, s3);

  // Allow the sensor output to settle after changing the filter.
  delayMicroseconds(300);

  // Measure the LOW pulse width.
  // Return 0 if no pulse is detected within 30 milliseconds.
  return pulseIn(SENSOR_OUT_PIN, LOW, 30000);
}


// Take several readings and calculate their average.
// Readings with a value of 0 are ignored.
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

  // Return 0 if no valid readings were obtained.
  if (validSamples == 0) {
    return 0;
  }

  return total / validSamples;
}


void setup() {
  // Start Serial communication for displaying sensor readings.
  Serial.begin(115200);

  // Configure the TCS3200 control pins.
  pinMode(S0_PIN, OUTPUT);
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);
  pinMode(S3_PIN, OUTPUT);
  pinMode(SENSOR_OUT_PIN, INPUT);

  // Configure the sensor illumination LED.
  pinMode(SENSOR_LED_PIN, OUTPUT);

  // Turn on the sensor illumination LED.
  // This matches the main project's setSensorLight(true) behaviour.
  digitalWrite(SENSOR_LED_PIN, HIGH);

  // Set the sensor output frequency scaling to 20%.
  digitalWrite(S0_PIN, HIGH);
  digitalWrite(S1_PIN, LOW);

  // Print CSV column headers for data collection.
  Serial.println("R_pulse,G_pulse,B_pulse");
}


void loop() {
  // Read the red filter pulse width.
  unsigned long redPulse =
    readAveragePulse(LOW, LOW);

  // Read the blue filter pulse width.
  unsigned long bluePulse =
    readAveragePulse(LOW, HIGH);

  // Read the green filter pulse width.
  unsigned long greenPulse =
    readAveragePulse(HIGH, HIGH);

  // Print the readings as one CSV row.
  // The column order is Red, Green, Blue.
  Serial.print(redPulse);
  Serial.print(",");

  Serial.print(greenPulse);
  Serial.print(",");

  Serial.println(bluePulse);

  // Wait one second before the next measurement.
  delay(1000);
}