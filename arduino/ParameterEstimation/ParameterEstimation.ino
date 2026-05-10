#define PULSE 1
#define STEP 0
#define DYNAMIC_BRAKING 1

// --- MOTOR & L298N PINS ---
const int ENA_PIN = 5;  // PWM speed control (Must be a squiggly '~' pin)
const int IN1_PIN = 7;  // Direction Pin 1
const int IN2_PIN = 6;  // Direction Pin 2

// --- ENCODER PINS ---
const int ENCODER_A = 2; // Phase A (Must be pin 2 for Hardware Interrupt)
const int ENCODER_B = 3; // Phase B (Direction check)

// --- MOTOR SPECIFICATIONS ---
const float TOTAL_PPR = 510; 

// --- VARIABLES ---
volatile long pulse_count = 0; 

unsigned long previous_time = 0;
const int time_window = 50; // Calculate RPM every 50 milliseconds

unsigned long pulse_previous_time =0;
unsigned long pulse_window = 2000;
bool isLow = 1;

void setup() {
  // 1. Start Serial Communication (Very fast for data logging)
  Serial.begin(115200);

  // 2. Configure Pins
  pinMode(ENA_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  // 3. Attach the Hardware Interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), updateEncoder, RISING);

  // 4. Print the CSV Header
  Serial.println("Time(ms),Volt,RPM");

  // 5. The Step Input! 
  // Set direction forward and slam the throttle to 100% (255)
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);
  if(STEP)
  {
    analogWrite(ENA_PIN, 255);
  }
  else if(PULSE)
  {
    analogWrite(ENA_PIN, 0);
  }

}

void loop() {
  unsigned long current_time = millis();
  unsigned long pulse_time = millis();
  
  // 1. DATA LOGGING WINDOW (Every 50ms)
  if (current_time - previous_time >= time_window) {
    
    // --- SAFE COPY ---
    noInterrupts();
    long current_pulses = pulse_count;
    pulse_count = 0; // Reset for the next 50ms window
    interrupts();
    // -----------------

    // --- RPM MATH ---
    float revolutions = (float)current_pulses / TOTAL_PPR;
    float rpm = revolutions * (60000.0 / time_window);

    // --- LOG DATA ---
    Serial.print(current_time);
    Serial.print(",");
    Serial.print(isLow?0:5);
    Serial.print(",");
    Serial.println(rpm);

    previous_time = current_time;
  }

  if(pulse_time - pulse_previous_time > pulse_window && PULSE)
  {
    if(isLow)
    {
        analogWrite(ENA_PIN, 255);
        if(DYNAMIC_BRAKING)
        {
          digitalWrite(IN1_PIN, HIGH);
          digitalWrite(IN2_PIN, LOW);
        }
        isLow = !isLow;
    }
    else
    {
      if(DYNAMIC_BRAKING)
      { 
        analogWrite(ENA_PIN, 255);   // keep bridge enabled for braking
        digitalWrite(IN1_PIN, LOW);
        digitalWrite(IN2_PIN, LOW);
      }
      else
      {
        analogWrite(ENA_PIN, 0);
      }
      isLow = !isLow; 
    }
    
    pulse_previous_time = pulse_time; 
  }
}

// ==========================================
// INTERRUPT SERVICE ROUTINE (ISR)
// ==========================================
void updateEncoder() {
    pulse_count++;
}