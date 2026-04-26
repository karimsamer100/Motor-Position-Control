// --- MOTOR & L298N PINS ---
const int ENA_PIN = 5;  // PWM speed control
const int IN1_PIN = 7;  // Direction Pin 1
const int IN2_PIN = 6;  // Direction Pin 2

// --- ENCODER PINS ---
const int ENCODER_A = 2; // Phase A interrupt pin
const int ENCODER_B = 3; // Phase B direction pin

// --- ENCODER SPECIFICATION ---
// UPDATED: Doubled to 1020 for CHANGE interrupt
const float TOTAL_PPR = 1020.0;   

// --- PID GAINS ---
float Kp = 0.04;
float Ki = 0.09;
float Kd = 0.00;

// --- CONTROL SETTINGS ---
float setpoint = 360.0;           // angle
const float maxVoltage = 12.0;   // saturation like Simulink
const int sample_time_ms = 20;

// --- VARIABLES ---
volatile long encoder_count = 0;

float error = 0;
float previous_error = 0;
float integral = 0;

unsigned long previous_time = 0;

void setup() {
  Serial.begin(115200);

  pinMode(ENA_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);

  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  // UPDATED: Changed from RISING to CHANGE to double resolution
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), updateEncoder, CHANGE);

  stopMotor();

  previous_time = millis();

  Serial.println("Time(ms),Setpoint,Angle,ControlVoltage,PWM,Error,Integra,Derivative");
}

void loop() {
  unsigned long current_time = millis();

  if (current_time - previous_time >= sample_time_ms) {
    float dt = (current_time - previous_time) / 1000.0;
    previous_time = current_time;

    noInterrupts();
    long count_copy = encoder_count;
    interrupts();

    // Convert encoder counts to angle
    float angle = (count_copy / TOTAL_PPR) * 360.0;

    // PID error
    error = setpoint - angle;

    // --- ANTI-WINDUP STRATEGY 1: Zero-Crossing Reset ---
    // If the error changes sign (we crossed the target), instantly erase the memory!
    if ((error > 0 && previous_error < 0) || (error < 0 && previous_error > 0)) {
      integral = 0;
    }

    float derivative = (error - previous_error) / dt;
    
    // Initialize outputs to 0 so they print correctly if the motor is stopped
    float controlVoltage = 0;
    int pwm = 0;

    // Stop near the target to reduce jitter
    if (abs(error) < 1.0) {
      stopMotor();
      // Notice the integral is NOT updated here. No deadband windup!
    } else {
      // --- INTEGRAL ACCUMULATION ---
      // Only build integral when actively correcting an error
      integral += error * dt;

      // --- ANTI-WINDUP STRATEGY 2: Integral Clamping ---
      // Prevent the memory from building up to dangerous levels (Cap at 3V)
      float max_integral_memory = 3.0 / Ki; 
      integral = constrain(integral, -max_integral_memory, max_integral_memory);

        // --- PID CALCULATION ---
        controlVoltage = (Kp * error) + (Ki * integral) + (Kd * derivative);

        // --- STICTION COMPENSATION ---
        // If the controller wants to move, instantly add the minimum voltage needed to break gear friction
        float stiction_voltage = 2.5; 
        
        if (controlVoltage > 0.05) {
          controlVoltage += stiction_voltage;
        } else if (controlVoltage < -0.05) {
          controlVoltage -= stiction_voltage;
        }

        // Limit voltage command
        controlVoltage = constrain(controlVoltage, -maxVoltage, maxVoltage);

      // Convert voltage command to PWM
      pwm = (int)((abs(controlVoltage) / maxVoltage) * 255.0);
      pwm = constrain(pwm, 0, 255);

      driveMotor(controlVoltage, pwm);
    }

    previous_error = error;

    // --- SERIAL LOGGING ---
    Serial.print(current_time);
    Serial.print(",");
    Serial.print(setpoint);
    Serial.print(",");
    Serial.print(angle);
    Serial.print(",");
    Serial.print(controlVoltage);
    Serial.print(",");
    Serial.print(pwm);
    Serial.print(",");
    Serial.print(error);
    Serial.print(",");
    Serial.print(integral);
    Serial.print(",");
    Serial.println(derivative);
  }
}

void updateEncoder() {
  // Read the current state of both encoder pins
  int stateA = digitalRead(ENCODER_A);
  int stateB = digitalRead(ENCODER_B);

  // Compare them using Quadrature Logic
  // If the states match, we are spinning one way. If they differ, the other.
  if (stateA == stateB) {
    encoder_count++;
  } else {
    encoder_count--;
  }
}

void driveMotor(float controlVoltage, int pwm) {
  if (controlVoltage > 0) {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
  } else {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);
  }

  analogWrite(ENA_PIN, pwm);
}

void stopMotor() {
  // True Dynamic Braking
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, HIGH);
  analogWrite(ENA_PIN, 255);
}