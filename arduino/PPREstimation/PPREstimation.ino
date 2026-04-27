// --- MOTOR & L298N PINS ---
const int PWM_PIN = 5;  // PWM speed control (Must be a squiggly '~' pin)
const int IN1_PIN = 7;  // Direction Pin 1
const int IN2_PIN = 6;  // Direction Pin 2

// --- Encoder Pins ---
const int ENCODER_A = 2; // Hardware Interrupt Pin
const int ENCODER_B = 3; 

volatile long pulseCount = 0; 
bool isRunning = true;

void setup() {
  // Start serial communication
  Serial.begin(9600);
  
  // Set motor pins as outputs
  pinMode(PWM_PIN, OUTPUT);
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  
  // Set encoder pins as inputs with internal pull-up resistors
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  
  // Attach the Hardware Interrupt
  // Every time Pin 2 goes from LOW to HIGH (RISING edge), it runs countPulse()
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), countPulse, RISING);
  
  // Start the motor at a slow, observable speed
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);
  analogWrite(PWM_PIN, 50); // Speed: 100 out of 255
  
  Serial.println("Motor is running...");
  Serial.println("Watch the shaft. Type ANY letter and hit ENTER when it does 1 full spin.");
}

void loop() {
  // Check if you typed something into the Serial Monitor
  if (Serial.available() > 0 && isRunning == true) {
    
    // 1. Instantly stop the motor
    analogWrite(PWM_PIN, 0);
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    
    // 2. Print the exact count
    Serial.println("\n--- MOTOR STOPPED ---");
    Serial.print("Pulses for 1 Revolution (CPR): ");
    Serial.println(pulseCount);

    // 3. Clear the serial buffer and lock the code so it doesn't run again
    while(Serial.available() > 0) {
      Serial.read();
      
    }
    isRunning = false; 
  }
}

// --- Interrupt Service Routine (ISR) ---
// This function interrupts the main code for a microsecond every time the encoder clicks
void countPulse() {
  pulseCount++;
}