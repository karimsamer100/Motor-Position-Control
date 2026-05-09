// Best Yet - Relative Angle + Direction + Zero Command
// With settling monitor after reaching target

// --- MOTOR & L298N PINS ---
const int ENA_PIN = 5;
const int IN1_PIN = 7;
const int IN2_PIN = 6;

// --- ENCODER PINS ---
const int ENCODER_A = 2;
const int ENCODER_B = 3;

// --- ENCODER SPECIFICATION ---
const float TOTAL_PPR = 995;

// --- PID GAINS ---
float Kp = 0.035;
float Ki = 0.4;
float Kd = 0.042;

// --- CONTROL SETTINGS ---
float setpoint = 0;
const float maxVoltage = 12.0;
const int sample_time_ms = 20;

const float targetTolerance = 0.8;

// --- STATE ---
bool commandReceived = false;
bool settlingMode = false;

unsigned long settlingStartTime = 0;
const unsigned long settlingDurationMs = 3000; // monitor 3 seconds after reaching target

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

  attachInterrupt(digitalPinToInterrupt(ENCODER_A), updateEncoder, CHANGE);

  stopMotor();
  previous_time = millis();

  Serial.println("Ready.");
  Serial.println("First move pointer to real 0 degrees, then type: z");
  Serial.println("Then enter: angle direction");
  Serial.println("Example: 90 0 -> move +90 from current position");
  Serial.println("Example: 90 1 -> move -90 from current position");
}

void loop() {
  if (!commandReceived && Serial.available() > 0) {
    char firstChar = Serial.peek();

    if (firstChar == 'z' || firstChar == 'Z') {
      Serial.read();

      noInterrupts();
      encoder_count = 0;
      interrupts();

      setpoint = 0;
      integral = 0;
      previous_error = 0;
      settlingMode = false;

      Serial.println("Current position is now ZERO.");
      Serial.println("Enter angle and direction, example: 90 0");
      clearSerialBuffer();
      return;
    }

    float inputAngle = Serial.parseFloat();
    int direction = Serial.parseInt();

    if (inputAngle > 0 && (direction == 0 || direction == 1)) {
      float currentAngle = getCurrentAngle();

      if (direction == 0) {
        setpoint = currentAngle + inputAngle;
      } else {
        setpoint = currentAngle - inputAngle;
      }

      Serial.print(inputAngle);
      Serial.print(" degrees to target ");
      Serial.println(setpoint);

      commandReceived = true;
      settlingMode = false;

      integral = 0;
      previous_error = 0;
      previous_time = millis();

      clearSerialBuffer();
    } else {
      Serial.println("Invalid input. Use: 90 0 or 90 1");
      clearSerialBuffer();
    }

    return;
  }

  if (!commandReceived) {
    stopMotor();
    return;
  }

  unsigned long current_time = millis();

  if (current_time - previous_time >= sample_time_ms) {
    float dt = (current_time - previous_time) / 1000.0;
    previous_time = current_time;

    float angle = getCurrentAngle();
    error = setpoint - angle;

    if (abs(error) < targetTolerance && !settlingMode) {
      stopMotor();
      settlingMode = true;
      settlingStartTime = millis();

      Serial.println("Target reached. Monitoring settling...");
    }

    if (settlingMode) {
      stopMotor();

      Serial.print("Target: ");
      Serial.print(setpoint);
      Serial.print(" | Angle: ");
      Serial.print(angle);
      Serial.print(" | Error: ");
      Serial.println(error);

      if (millis() - settlingStartTime >= settlingDurationMs) {
        settlingMode = false;
        commandReceived = false;

        Serial.println("Movement finished.");
        Serial.println("Enter another relative command, or type z to reset zero.");
      }

      return;
    }

    float derivative = (error - previous_error) / dt;

    integral += error * dt;

    float max_integral_memory = 3.0 / Ki;
    integral = constrain(integral, -max_integral_memory, max_integral_memory);

    float controlVoltage = (Kp * error) + (Ki * integral) + (Kd * derivative);

    controlVoltage = constrain(controlVoltage, -maxVoltage, maxVoltage);

    int pwm = (int)((abs(controlVoltage) / maxVoltage) * 255.0);
    pwm = constrain(pwm, 0, 255);

    if (pwm > 0 && pwm < 45) {
      pwm = 45;
    }

    driveMotor(controlVoltage, pwm);

    previous_error = error;

    Serial.print("Target: ");
    Serial.print(setpoint);
    Serial.print(" | Angle: ");
    Serial.print(angle);
    Serial.print(" | Error: ");
    Serial.println(error);
  }
}

float getCurrentAngle() {
  noInterrupts();
  long count_copy = encoder_count;
  interrupts();

  return (count_copy / TOTAL_PPR) * 360.0;
}

void updateEncoder() {
  int stateA = digitalRead(ENCODER_A);
  int stateB = digitalRead(ENCODER_B);

  if (stateA == stateB) {
    encoder_count--;
  } else {
    encoder_count++;
  }
}

void driveMotor(float controlVoltage, int pwm) {
  if (controlVoltage > 0) {
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, HIGH);
  } else {
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
  }

  analogWrite(ENA_PIN, pwm);
}

void stopMotor() {
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  analogWrite(ENA_PIN, 255);
}

void clearSerialBuffer() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}