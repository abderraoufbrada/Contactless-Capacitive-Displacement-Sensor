

#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define DISCHARGE_PIN 26
#define SIGNAL_PIN    27
#define SDA_PIN       21
#define SCL_PIN       22

LiquidCrystal_I2C lcd(0x27, 16, 2);


const float R_CHARGE = 100000.0;
const float THRESHOLD_FACTOR = 0.693147;

const unsigned long DISCHARGE_TIME_US = 1000;
const unsigned long TIMEOUT_US = 1000000;
const unsigned long CYCLE_INTERVAL_MS = 500;

// =================================
// Distance calculation (PCHIP piecewise)
// x = capacitance in nF
// return = distance in mm
// =================================
float calculateDistance(float x) {
  if (x <= 0.577f) {
    return 11.0f;
  }
  else if (x < 0.592f) {
    float t = x - 0.577f;
    return ((608.257985f * t + -173.327482f) * t + -64.2036125f) * t + 11.0f;
  }
  else if (x < 0.606f) {
    float t = x - 0.592f;
    return ((12427.0848f * t + -347.958374f) * t + -68.9928628f) * t + 10.0f;
  }
  else if (x < 0.62f) {
    float t = x - 0.606f;
    return ((12427.0848f * t + -173.979187f) * t + -71.4285714f) * t + 9.0f;
  }
  else if (x < 0.635f) {
    float t = x - 0.62f;
    return ((76881.8568f * t + -998.148109f) * t + -68.9928628f) * t + 8.0f;
  }
  else if (x < 0.664f) {
    float t = x - 0.635f;
    return ((-15657.3781f * t + 887.143074f) * t + -47.0420527f) * t + 7.0f;
  }
  else if (x < 0.692f) {
    float t = x - 0.664f;
    return ((19301.1851f * t + -562.681975f) * t + -35.0913195f) * t + 6.0f;
  }
  else if (x < 0.765f) {
    float t = x - 0.692f;
    return ((-1426.38106f * t + 206.954484f) * t + -21.2051228f) * t + 5.0f;
  }
  else if (x < 0.837f) {
    float t = x - 0.765f;
    return ((842.799196f * t + -62.0088582f) * t + -13.7933221f) * t + 4.0f;
  }
  else if (x < 0.981f) {
    float t = x - 0.837f;
    return ((94.4584037f * t + 4.9461855f) * t + -9.61538462f) * t + 3.0f;
  }
  else {
    return 2.0f;
  }
}


void setup() {
  Serial.begin(115200);

  pinMode(DISCHARGE_PIN, OUTPUT);
  pinMode(SIGNAL_PIN, INPUT);
  digitalWrite(DISCHARGE_PIN, HIGH);

  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Electronic Ruler");
  Serial.println("Capacitive Displacement Sensor");
  Serial.println("------------------------------");

  delay(1500);
  lcd.clear();
}

// =================================
// Loop
// =================================
void loop() {
  // 1. Discharge capacitor
  digitalWrite(DISCHARGE_PIN, HIGH);
  delayMicroseconds(DISCHARGE_TIME_US);

  // 2. Start charging
  digitalWrite(DISCHARGE_PIN, LOW);

  unsigned long startTime = micros();
  bool detected = false;

  // 3. Wait for threshold crossing
  while (micros() - startTime < TIMEOUT_US) {
    if (digitalRead(SIGNAL_PIN) == LOW) {
      detected = true;
      break;
    }
  }

  unsigned long elapsedTime = micros() - startTime;

  if (detected && elapsedTime > 0) {
    // 4. Calculate capacitance
    float timeSeconds    = elapsedTime * 1e-6;
    float capacitanceF   = timeSeconds / (THRESHOLD_FACTOR * R_CHARGE);
    float capacitance_nF = capacitanceF * 1e9;

    // 5. Calculate distance from the capacitance (FIXED)
    float distance_mm = calculateDistance(capacitance_nF);

    // 6. Serial output
    Serial.print("C = ");
    Serial.print(capacitance_nF, 3);
    Serial.print(" nF | t = ");
    Serial.print(elapsedTime);
    Serial.print(" us | Distance = ");
    Serial.print(distance_mm, 2);
    Serial.println(" mm");

    // 7. LCD output
    lcd.setCursor(0, 0);
    lcd.print("C:");
    lcd.print(capacitance_nF, 3);
    lcd.print("nF      ");

    lcd.setCursor(0, 1);
    lcd.print("Dist:");
    lcd.print(distance_mm, 2);
    lcd.print("mm   ");
  } else {
    Serial.println("Timeout: threshold not detected");

    lcd.setCursor(0, 0);
    lcd.print("Timeout         ");
    lcd.setCursor(0, 1);
    lcd.print("No signal       ");
  }

  delay(CYCLE_INTERVAL_MS);
}
