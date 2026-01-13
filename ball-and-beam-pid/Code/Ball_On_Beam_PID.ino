#include <Servo.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================
//   BALL-AND-BEAM PRO - v3.1 (JOYSTICK EDITION)
// ============================================

// --- PIN DEFINITIONS ---
const int SERVO_PIN = 30;
const int ENABLE_BTN_PIN = 4;  // Master Run/Pause

// --- JOYSTICK PINS ---
const int JOY_X_PIN = A0;      // VRx
const int JOY_Y_PIN = A1;      // VRy
const int JOY_SW_PIN = 5;      // SW (Button)

// --- OLED DISPLAY ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- SENSORS & MOTORS ---
VL53L0X sensor;
Servo servo;

// --- STATISTICS (Restored for Debugging) ---
int sensorFailures = 0;
int jumpRejections = 0;
double maxError = 0;
double errorSum = 0;
int errorCount = 0;

// --- SYSTEM CONSTANTS ---
const float BEAM_LENGTH_CM = 32.2;
const float VALID_MIN_DIST = 1.5;
const float VALID_MAX_DIST = 33.7;
const float SERVO_FLAT_ANGLE = 124;
const int SERVO_MIN = 25;
const int SERVO_MAX = 180;
const double MAX_JUMP_CM = 6.0;

// --- DEFAULTS (For Reset) ---
const double DEF_KP = 6.2;
const double DEF_KI = 0.15;
const double DEF_KD = 3.1;
const double DEF_TGT = 18.0;

// --- TUNABLE VARIABLES ---
double Kp = DEF_KP;
double Ki = DEF_KI;
double Kd = DEF_KD;
double targetDist = DEF_TGT;
double DEADBAND_CM = 1.0;

// --- PID VARIABLES ---
double input, output;
double lastInput = 0;
double iTerm = 0;
double lastDTerm = 0;
const double D_FILTER = 0.6;
const int SAMPLE_TIME = 35; // PID Loop Time
const double OUTPUT_MIN = -60.0;
const double OUTPUT_MAX = 60.0;

// --- SENSOR FILTERING ---
const int numSamples = 7;
int sampleIndex = 0;
double samples[numSamples];
double averageDist = 0;
double lastRawDist = 0;

// --- STATE VARIABLES ---
bool systemActive = false;
int controlLoops = 0;
unsigned long lastPIDTime = 0;
unsigned long lastDisplayTime = 0;
unsigned long lastDebounceTime = 0;
int lastEnableBtnState = HIGH;

// --- MENU SYSTEM VARIABLES ---
// 0=Dashboard, 1=Menu List, 2=Editing Value
int uiState = 0; 
int menuIndex = 0; // 0=Kp, 1=Ki, 2=Kd, 3=Tgt, 4=Reset, 5=Exit
const int MENU_ITEMS = 6;
unsigned long lastJoyBtnTime = 0;

// --- JOYSTICK VARIABLES ---
unsigned long lastJoyMoveTime = 0;
const int JOY_THRESHOLD_HIGH = 800; // Stick pushed far
const int JOY_THRESHOLD_LOW = 200;  // Stick pushed far other way


void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);

  // 1. PIN SETUP
  pinMode(ENABLE_BTN_PIN, INPUT_PULLUP);
  pinMode(JOY_SW_PIN, INPUT_PULLUP);
  // Analog pins A0/A1 are inputs by default

  // 3. INIT DISPLAY
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,20);
  display.println("Booting System...");
  display.display();

  // 4. INIT SENSOR
  sensor.setTimeout(500);
  if (!sensor.init()) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("SENSOR FAILED!");
    display.display();
    while (1);
  }
  sensor.setMeasurementTimingBudget(33000);
  sensor.startContinuous();

  // 5. INIT SERVO
  servo.attach(SERVO_PIN);
  servo.write(SERVO_FLAT_ANGLE);

  // 6. INIT FILTER
  for (int i = 0; i < numSamples; i++) samples[i] = targetDist;
  averageDist = targetDist;
  lastRawDist = targetDist;
  lastInput = targetDist;

  delay(1000);
  lastPIDTime = millis();
}

void loop() {
  unsigned long currentMillis = millis();

  // =========================
  // 1. READ INPUTS
  // =========================
  
  // -- ENABLE BUTTON --
  int btnRead = digitalRead(ENABLE_BTN_PIN);
  if (btnRead == LOW && lastEnableBtnState == HIGH && (currentMillis - lastDebounceTime > 200)) {
    systemActive = !systemActive;
    lastDebounceTime = currentMillis;
    if (systemActive) {
      // Reset logic on start
      iTerm = 0; 
      lastDTerm = 0; 
      lastInput = averageDist;
      
      // Reset stats on start
      maxError = 0;
      errorSum = 0;
      errorCount = 0;
      sensorFailures = 0;
      jumpRejections = 0;
    }
  }
  lastEnableBtnState = btnRead;

  // -- JOYSTICK BUTTON --
  if (digitalRead(JOY_SW_PIN) == LOW && (currentMillis - lastJoyBtnTime > 250)) {
    handleMenuClick();
    lastJoyBtnTime = currentMillis;
  }

  // -- JOYSTICK MOVEMENT --
  handleJoystick();

  // =========================
  // 2. PID LOOP (35ms)
  // =========================
  if (currentMillis - lastPIDTime >= SAMPLE_TIME) {
    double timeChange = (currentMillis - lastPIDTime) / 1000.0;
    lastPIDTime = currentMillis;
    controlLoops++;

    // --- SENSOR READING ---
    uint16_t dist_mm = sensor.readRangeContinuousMillimeters();
    double distance = dist_mm / 10.0;

    bool valid = true;
    if (sensor.timeoutOccurred() || distance < VALID_MIN_DIST || distance > VALID_MAX_DIST) {
      valid = false;
      sensorFailures++; // Count failure
    }
    if (valid && abs(distance - lastRawDist) > MAX_JUMP_CM) {
      valid = false;
      jumpRejections++; // Count jump
    }

    if (valid) lastRawDist = distance;
    else distance = averageDist;

    // Moving Avg
    samples[sampleIndex] = distance;
    sampleIndex = (sampleIndex + 1) % numSamples;
    double sum = 0;
    for (int i=0; i<numSamples; i++) sum += samples[i];
    averageDist = sum / numSamples;

    // --- CONTROL LOGIC ---
    // Initialize these HERE so the print block can see them
    int servoAngle = SERVO_FLAT_ANGLE;
    double error = targetDist - averageDist; 
    String status = "";

    if (systemActive) {
      input = averageDist;
      
      // Update Stats
      if (abs(error) > maxError) maxError = abs(error);
      errorSum += abs(error);
      errorCount++;
      
      if (abs(error) <= DEADBAND_CM) {
        // Deadband
        output = 0; iTerm = 0; lastDTerm = 0;
        servoAngle = SERVO_FLAT_ANGLE;
        status = "FLAT OK";
      } else {
        // PID
        double pTerm = Kp * error;
        iTerm += Ki * error * timeChange;
        iTerm = constrain(iTerm, -OUTPUT_MAX/2, OUTPUT_MAX/2);
        
        double dInput = (input - lastInput) / timeChange;
        double dTerm = D_FILTER * lastDTerm - Kd * (1 - D_FILTER) * dInput;
        lastDTerm = dTerm;

        output = pTerm + iTerm + dTerm;
        output = constrain(output, OUTPUT_MIN, OUTPUT_MAX);
        servoAngle = SERVO_FLAT_ANGLE - output;
        servoAngle = constrain(servoAngle, SERVO_MIN, SERVO_MAX);
        
        if (abs(output) > OUTPUT_MAX * 0.9) status = "! SAT";
        else status = "CTRL";
      }
      lastInput = input;
    } else {
      // Paused
      lastInput = averageDist;
      output = 0;
      status = "PAUSED";
    }

    servo.write(servoAngle);

    // =========================
    // SERIAL DEBUG (Restored!)
    // =========================
    if (controlLoops % 5 == 0) {
       Serial.print(averageDist, 1);
       Serial.print(" | ");
       Serial.print(targetDist, 1);
       Serial.print(" | ");
       Serial.print(error, 1);
       Serial.print(" | P:");
       Serial.print(Kp * error, 1);
       Serial.print(" I:");
       Serial.print(iTerm, 1);
       Serial.print(" D:");
       Serial.print(lastDTerm, 1);
       Serial.print(" | ");
       Serial.print(output, 1);
       Serial.print(" | ");
       Serial.print(servoAngle);
       Serial.print(" | ");
       Serial.println(status);
    }
    
    // Stats every 500 loops
    if (controlLoops % 500 == 0) {
       Serial.println();
       Serial.println("--- STATS ---");
       Serial.print("Loops: "); Serial.print(controlLoops);
       Serial.print(" | Sensor Fails: "); Serial.print(sensorFailures);
       Serial.print(" | Jump Rejects: "); Serial.println(jumpRejections);
       Serial.print("Max Error: "); Serial.print(maxError, 1);
       if(errorCount > 0) {
         Serial.print(" cm | Avg Error: "); Serial.print(errorSum / errorCount, 2); Serial.println(" cm");
       } else {
         Serial.println(" cm | Avg Error: 0.00 cm");
       }
       Serial.println("-------------");
       Serial.println();
    }
  }

  // =========================
  // 3. DISPLAY UPDATE (150ms)
  // =========================
  if (currentMillis - lastDisplayTime >= 150) {
    updateDisplay();
    lastDisplayTime = currentMillis;
  }
}

// --- NEW JOYSTICK LOGIC ---

void handleJoystick() {
  unsigned long now = millis();
  
  // Throttle speed (only move every 200ms)
  if (now - lastJoyMoveTime < 200) return; 

  int xVal = analogRead(JOY_X_PIN);
  int yVal = analogRead(JOY_Y_PIN);
  bool moved = false;

  // --- Y-AXIS: MENU NAVIGATION (Up/Down) ---
  // Assuming Y < 200 is UP, Y > 800 is DOWN
  if (yVal < JOY_THRESHOLD_LOW) {
    if (uiState == 1) { 
      menuIndex--; 
      if (menuIndex < 0) menuIndex = MENU_ITEMS - 1;
      moved = true;
    }
  }
  else if (yVal > JOY_THRESHOLD_HIGH) {
    if (uiState == 1) {
      menuIndex++;
      if (menuIndex >= MENU_ITEMS) menuIndex = 0;
      moved = true;
    }
  }

  // --- X-AXIS: EDIT VALUES (Left/Right) ---
  // Only active when editing a parameter (uiState == 2)
  if (uiState == 2) {
    int direction = 0;
    if (xVal < JOY_THRESHOLD_LOW) direction = -1; // Left (Decrease)
    if (xVal > JOY_THRESHOLD_HIGH) direction = 1; // Right (Increase)

    if (direction != 0) {
      if (menuIndex == 0) Kp += direction * 0.1;
      if (menuIndex == 1) Ki += direction * 0.01;
      if (menuIndex == 2) Kd += direction * 0.1;
      if (menuIndex == 3) targetDist += direction * 0.5;
      moved = true;
    }
  }

  if (moved) {
    lastJoyMoveTime = now;
    updateDisplay(); // Force instant update
  }
}

void handleMenuClick() {
  if (uiState == 0) {
    uiState = 1; // Go to Menu
  }
  else if (uiState == 1) {
    if (menuIndex == 5) { // Exit
      uiState = 0; 
    } 
    else if (menuIndex == 4) { // Reset
      Kp = DEF_KP; Ki = DEF_KI; Kd = DEF_KD; targetDist = DEF_TGT;
      uiState = 0; // Go back to Dash
    } 
    else {
      uiState = 2; // Edit Parameter
    }
  }
  else if (uiState == 2) {
    uiState = 1; // Save & Back to Menu
  }
  updateDisplay();
}

void updateDisplay() {
  display.clearDisplay();
  
  if (uiState == 0) {
    // --- DASHBOARD ---
    display.setTextSize(1);
    display.setCursor(0,0);
    if (systemActive) display.print("STATUS: >> RUNNING");
    else              display.print("STATUS: || PAUSED");
    
    display.setCursor(0,16);
    display.print("DIST: "); display.print(averageDist, 1); display.print(" cm");
    
    display.setCursor(0,27);
    display.print("TARG: "); display.print(targetDist, 1); display.print(" cm");

    // Visual Error Bar
    display.drawRect(10, 45, 108, 10, WHITE); // Frame
    display.drawFastVLine(64, 43, 14, WHITE); // Center mark
    
    double err = averageDist - targetDist;
    int barLen = constrain(err * 3, -50, 50); // Scale error for pixels
    if (barLen > 0) display.fillRect(64, 47, barLen, 6, WHITE);
    else display.fillRect(64 + barLen, 47, -barLen, 6, WHITE);
  } 
// ... inside updateDisplay() ...

  else {
    // --- MENU / EDIT ---
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println(uiState == 1 ? "-- SETTINGS --" : "-- EDITING --");
    
    const char* labels[] = {"Kp", "Ki", "Kd", "Target", "RESET", "EXIT"};
    
    for (int i=0; i<MENU_ITEMS; i++) {
      int y = 12 + (i * 9); // Spacing
      
      // *** FIX IS HERE *** // Force the cursor to the start of the new line BEFORE printing the arrow
      display.setCursor(0, y); 
      
      if (i == menuIndex) display.print(">"); // Cursor
      else display.print(" ");
      
      display.setCursor(10, y); // Move over slightly for the text
      display.print(labels[i]);
      
      // Show values next to parameters
      display.setCursor(70, y);
      if (i==0) display.print(Kp, 1);
      if (i==1) display.print(Ki, 2);
      if (i==2) display.print(Kd, 1);
      if (i==3) display.print(targetDist, 1);
    }
  }
  
  display.display();
}