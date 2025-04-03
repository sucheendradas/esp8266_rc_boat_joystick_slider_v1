/*============ BLYNK CONFIGURATION ============*/
// Blynk template information (from your IoT dashboard)
#define BLYNK_TEMPLATE_ID "TMPL3l-s3Bs7t"
#define BLYNK_TEMPLATE_NAME "RC boat esp8266"
#define BLYNK_AUTH_TOKEN "gbT9odNuyrO_mez7GzXKm1wJ1RWiWNnq"

/*============ NETWORK CREDENTIALS ============*/
const char* ssid = "eDragon";       // Wi-Fi network name
const char* pass = "$123@Mac";      // Wi-Fi password

/*============ MOTOR PIN DEFINITIONS ============*/
// Motor control pins (using NodeMCU GPIO numbering)
#define MOTOR_A_IN1 D2  // Left motor forward signal
#define MOTOR_A_IN2 D5  // Left motor reverse signal
#define MOTOR_B_IN3 D6  // Right motor forward signal
#define MOTOR_B_IN4 D7  // Right motor reverse signal

// Add these constants at top of code
#define MIN_SPEED 50    // Minimum effective PWM value (motors won't spin below this)
#define MAX_SPEED 220   // Safety limit to prevent full-battery speed
#define SPEED_STEPS 5   // Number of speed increments (for non-linear control)

/*============ LIBRARY INCLUDES ============*/
#include <ESP8266WiFi.h>       // ESP8266 core WiFi library
#include <BlynkSimpleEsp8266.h>// Blynk IoT platform integration

/*============ CONTROL VARIABLES ============*/
int boatSpeed = 0;             // Current speed (0-255 PWM value)
int steeringValue = 127;       // Steering position (0-254, 127=center)
bool isForward = true;         // Direction flag (true=forward, false=reverse)
bool emergencyStop = false;    // Emergency stop override flag

/*============ MOTOR CONTROL FUNCTION ============*/
void updateMotors() {
  // Emergency stop takes priority over all controls
  if (emergencyStop) {
    // Cut power to all motor drivers immediately
    analogWrite(MOTOR_A_IN1, 0);
    analogWrite(MOTOR_A_IN2, 0);
    analogWrite(MOTOR_B_IN3, 0);
    analogWrite(MOTOR_B_IN4, 0);
    return; // Exit function without processing other inputs
  }

  // Calculate base motor speeds
  boatSpeed = 
  int leftSpeed = boatSpeed;
  int rightSpeed = boatSpeed;

  /*------ STEERING ADJUSTMENTS ------*/
  // Right turn logic (steeringValue > center position)
  if (steeringValue > 127) {
    // Calculate right turn intensity (0-127 range)
    int rightAmount = steeringValue - 127;
    // Reduce left motor speed proportionally
    leftSpeed = boatSpeed - (boatSpeed * rightAmount) / 128;
  }
  // Left turn logic (steeringValue < center position)
  else if (steeringValue < 127) {
    // Calculate left turn intensity (0-127 range)
    int leftAmount = 127 - steeringValue;
    // Reduce right motor speed proportionally
    rightSpeed = boatSpeed - (boatSpeed * leftAmount) / 127;
  }

  /*------ SPEED SAFETY LIMITS ------*/
  // Ensure PWM values stay within valid 0-255 range
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  /*------ MOTOR DIRECTION CONTROL ------*/
  if (isForward) {
    // Forward movement: activate IN1/IN3 pins, disable reverse pins
    analogWrite(MOTOR_A_IN1, leftSpeed);
    analogWrite(MOTOR_A_IN2, 0);
    analogWrite(MOTOR_B_IN3, rightSpeed);
    analogWrite(MOTOR_B_IN4, 0);
  } else {
    // Reverse movement: activate IN2/IN4 pins, disable forward pins
    analogWrite(MOTOR_A_IN1, 0);
    analogWrite(MOTOR_A_IN2, leftSpeed);
    analogWrite(MOTOR_B_IN3, 0);
    analogWrite(MOTOR_B_IN4, rightSpeed);
  }
}

/*============ BLYNK CONTROL HANDLERS ============*/

// Steering input (Horizontal slider on V1)
BLYNK_WRITE(V1) { 
  if (emergencyStop) return; // Ignore input during emergency stop
  steeringValue = param.asInt(); // Update steering position (0-254)
  updateMotors(); // Apply changes immediately
}

// Speed input (Vertical slider on V2)
BLYNK_WRITE(V2) {
  if (emergencyStop) return;
  
  // Original linear mapping
  // boatSpeed = map(param.asInt(), 0, 255, MIN_SPEED, MAX_SPEED);
  
  // Non-linear mapping (quadratic curve for better low-speed control)
  float normalized = param.asInt() / 255.0;
  boatSpeed = (int)(MIN_SPEED + (MAX_SPEED - MIN_SPEED) * normalized * normalized);
  
  updateMotors();
}
// Direction toggle (Switch on V3)
BLYNK_WRITE(V3) {
  if (emergencyStop) return; // Ignore input during emergency stop
  isForward = param.asInt(); // Update direction (1=forward, 0=reverse)
  updateMotors(); // Apply changes immediately
}

// Emergency Stop (Switch on V4)
BLYNK_WRITE(V4) {
  if (param.asInt()) { // When button is pressed (HIGH)
    emergencyStop = true; // Activate emergency stop flag
    boatSpeed = 0; // Reset speed value
    steeringValue = 127; // Center steering
    isForward = true; // Set default direction to forward
    
    // Update Blynk app UI to reflect stopped state
    Blynk.virtualWrite(V1, 127); // Center steering slider
    Blynk.virtualWrite(V2, 0);   // Reset speed slider to 0
    Blynk.virtualWrite(V3, 1);   // Set direction switch to forward
    updateMotors(); // Execute full stop
  } else { // When button is released (LOW)
    emergencyStop = false; // Clear emergency state
    // Synchronize with current widget states
    Blynk.syncVirtual(V1, V2, V3); // Get latest values from app
  }
}

/*============ ARDUINO STANDARD FUNCTIONS ============*/

void setup() {
  // Configure PWM range for motor control
  analogWriteRange(255); // Set 8-bit resolution (0-255)
  
  // Initialize motor control pins
  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN3, OUTPUT);
  pinMode(MOTOR_B_IN4, OUTPUT);

  // Start Blynk IoT connection
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop() {
  // Maintain Blynk connection and process incoming data
  Blynk.run();
}