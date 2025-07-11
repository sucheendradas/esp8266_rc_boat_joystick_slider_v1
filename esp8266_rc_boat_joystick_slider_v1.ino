
//NodeMCU 1.0 (ESP - 12E MODULE )
//Blink LED TEST - LED_BUILTIN
#define BLYNK_TEMPLATE_ID "TMPL3E97YKwyN"
#define BLYNK_TEMPLATE_NAME "Rcboat"
#define BLYNK_AUTH_TOKEN "CE28b_Olvnooa5LHd2755pyZss3gBSJo"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ArduinoOTA.h>

//hotspot 
const char* ssid = "eDragon";
const char* pass = "$123@Mac";

// Motor control pins
#define MOTOR_A_IN1 D2  // Left motor forward
#define MOTOR_A_IN2 D5  // Left motor reverse
#define MOTOR_B_IN3 D6  // Right motor forward
#define MOTOR_B_IN4 D7  // Right motor reverse

int throttle = 0;      // Vertical slider (0-1024)//350 to 675
int steering = 512;    // Horizontal slider (0-1024, center = straight)
float steeringOffsetScale = 0.4;  // Steering sensitivity (0.0 to 0.5)

// Calculate motor speed multiplier based on steering and side
float calculateMotorFactor(int steerValue, bool isLeftMotor) {
  float steerOffset = (steerValue - 512) / 512.0;

  float leftFactor = 0.5;
  float rightFactor = 0.5;

  if (steerOffset > 0) {
    // Right turn
    leftFactor = 0.5 + 0.5 * steerOffset;
    rightFactor = 0.5 - steeringOffsetScale * steerOffset;  // reduced from 0.5 to min 10%
  } else if (steerOffset < 0) {
    // Left turn
    leftFactor = 0.5 + steeringOffsetScale * steerOffset;
    rightFactor = 0.5 - 0.5 * steerOffset;
  }

  return isLeftMotor ? leftFactor : rightFactor;
}

void updateMotors() {
  int baseSpeed = constrain(throttle, 0, 1023);

  float leftFactor = calculateMotorFactor(steering, true);
  float rightFactor = calculateMotorFactor(steering, false);

  int leftSpeed = constrain(baseSpeed * leftFactor, 0, 1023);
  int rightSpeed = constrain(baseSpeed * rightFactor, 0, 1023);

  analogWrite(MOTOR_A_IN1, leftSpeed);
  analogWrite(MOTOR_A_IN2, 0);
  analogWrite(MOTOR_B_IN3, rightSpeed);
  analogWrite(MOTOR_B_IN4, 0);

  // 🔍 Print motor speeds to Serial Monitor
  Serial.print("Throttle: ");
  Serial.print(baseSpeed);
  Serial.print(" | Steering: ");
  Serial.print(steering);
  Serial.print(" | Left Speed: ");
  Serial.print(leftSpeed);
  Serial.print(" | Right Speed: ");
  Serial.println(rightSpeed);
}


BLYNK_WRITE(V1) { // Vertical Slider
  throttle = param.asInt();
  updateMotors();
}

BLYNK_WRITE(V0) {// Horizontal Slider
  steering = param.asInt();
  updateMotors();
}

BLYNK_WRITE(V2) {//Horizontal Slider
  steeringOffsetScale = param.asInt() / 100.0;  // Slider , integer 0–50 → scale 0.0 to 0.5
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(MOTOR_A_IN1, OUTPUT);
  pinMode(MOTOR_A_IN2, OUTPUT);
  pinMode(MOTOR_B_IN3, OUTPUT);
  pinMode(MOTOR_B_IN4, OUTPUT);

  // OTA setup
  ArduinoOTA.setHostname("RC-Boat"); // Optional name
  ArduinoOTA.begin();
}

void loop() {
  Blynk.run();
  ArduinoOTA.handle(); // Important!
}
