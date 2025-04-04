#define BLYNK_TEMPLATE_ID "TMPL3l-s3Bs7t"
#define BLYNK_TEMPLATE_NAME "RC boat esp8266"
#define BLYNK_AUTH_TOKEN "gbT9odNuyrO_mez7GzXKm1wJ1RWiWNnq"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

const char* ssid = "eDragon";
const char* pass = "$123@Mac";

// Motor control pins
#define MOTOR_A_IN1 D2  // Left motor forward
#define MOTOR_A_IN2 D5  // Left motor reverse
#define MOTOR_B_IN3 D6  // Right motor forward
#define MOTOR_B_IN4 D7  // Right motor reverse

int throttle = 0;  // Vertical slider (0-1024)
int steering = 0;  // Horizontal slider (0-1024)

// Function to update motor speeds based on slider inputs
void updateMotors() {
    int baseSpeed = constrain(throttle, 0, 1023);  // Ensure valid range
    int steeringOffset = constrain(steering - 512, -512, 512); // Convert to -512 to 512

    int leftSpeed = constrain(baseSpeed - steeringOffset, 0, 1023);
    int rightSpeed = constrain(baseSpeed + steeringOffset, 0, 1023);

    analogWrite(MOTOR_A_IN1, leftSpeed);
    analogWrite(MOTOR_A_IN2, 0);
    analogWrite(MOTOR_B_IN3, rightSpeed);
    analogWrite(MOTOR_B_IN4, 0);
}

// Blynk vertical slider (V2) - Throttle control
BLYNK_WRITE(V2) {
    throttle = param.asInt();
    updateMotors();
}

// Blynk horizontal slider (V1) - Steering control
BLYNK_WRITE(V1) {
    steering = param.asInt();
    updateMotors();
}

void setup() {
    Serial.begin(115200);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    
    pinMode(MOTOR_A_IN1, OUTPUT);
    pinMode(MOTOR_A_IN2, OUTPUT);
    pinMode(MOTOR_B_IN3, OUTPUT);
    pinMode(MOTOR_B_IN4, OUTPUT);
}

void loop() {
    Blynk.run();
}
