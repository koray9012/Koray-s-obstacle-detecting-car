// ---------- PINS ----------
#define TRIG_L 5
#define ECHO_L 4
#define TRIG_R 6
#define ECHO_R 7

#define IN1 11
#define IN2 10
#define IN3 9
#define IN4 8

#define EN 3 // Both ENA and ENB connected here

// ---------- SETTINGS ----------
#define DANGER_MIN 40    // cm
#define WAIT_TIME 2000   // ms pause between steps
#define BACKWARD_TIME 500
#define TURN90_TIME 400  // approx 90° turn
#define TURN180_TIME 800 // approx 180° turn
#define LOOP_DELAY 20    // ms loop delay
#define SPEED 89        // 0-255 for PWM, lower is slower

// ---------- STATE MACHINE ----------
enum DangerState { IDLE, STOP1, BACKWARD, STOP2, TURN, STOP3, FORWARD };
DangerState dangerState = IDLE;
unsigned long stateStartTime = 0;

// Store which sensor triggered danger
bool dangerLeftDetected = false;
bool dangerRightDetected = false;

// ---------- FUNCTIONS ----------
long readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 15000);
  if(duration == 0) return 100;
  return duration * 0.034 / 2;
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void forward() {
  analogWrite(EN, SPEED); // set speed
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void backward() {
  analogWrite(EN, SPEED);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  analogWrite(EN, SPEED);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
  analogWrite(EN, SPEED);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// ---------- SETUP ----------
void setup() {
  pinMode(TRIG_L, OUTPUT);
  pinMode(ECHO_L, INPUT);
  pinMode(TRIG_R, OUTPUT);
  pinMode(ECHO_R, INPUT);
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(EN, OUTPUT);
  
  stopMotors();
  Serial.begin(9600);
}

// ---------- LOOP ----------
void loop() {
  unsigned long currentTime = millis();
  
  // Only check sensors if not already in a danger sequence
  if(dangerState == IDLE) {
    long distL = readDistance(TRIG_L, ECHO_L);
    long distR = readDistance(TRIG_R, ECHO_R);
    
    bool dangerL = distL >= DANGER_MIN;
    bool dangerR = distR >= DANGER_MIN;
    
    if(dangerL || dangerR){
      dangerLeftDetected  = dangerL;
      dangerRightDetected = dangerR;
      
      stopMotors();
      dangerState = STOP1;
      stateStartTime = currentTime;
    } else {
      forward();
    }
  }

  // State machine for full danger sequence
  switch(dangerState) {
    case STOP1:
      if(currentTime - stateStartTime >= WAIT_TIME){
        backward();
        dangerState = BACKWARD;
        stateStartTime = currentTime;
      }
      break;

    case BACKWARD:
      if(currentTime - stateStartTime >= BACKWARD_TIME){
        stopMotors();
        dangerState = STOP2;
        stateStartTime = currentTime;
      }
      break;

    case STOP2:
      if(currentTime - stateStartTime >= WAIT_TIME){
        dangerState = TURN;
        stateStartTime = currentTime;
      }
      break;

    case TURN:
      // Turn according to stored sensor state
      if(dangerLeftDetected && dangerRightDetected) {
        turnLeft(); // 180° turn
        if(currentTime - stateStartTime >= TURN180_TIME){
          stopMotors();
          dangerState = STOP3;
          stateStartTime = currentTime;
        }
      }
      else if(dangerLeftDetected) {
        turnRight(); // 90° turn
        if(currentTime - stateStartTime >= TURN90_TIME){
          stopMotors();
          dangerState = STOP3;
          stateStartTime = currentTime;
        }
      }
      else if(dangerRightDetected) {
        turnLeft(); // 90° turn
        if(currentTime - stateStartTime >= TURN90_TIME){
          stopMotors();
          dangerState = STOP3;
          stateStartTime = currentTime;
        }
      }
      break;

    case STOP3:
      if(currentTime - stateStartTime >= WAIT_TIME){
        forward();
        dangerState = FORWARD;
        stateStartTime = currentTime;
      }
      break;

    case FORWARD:
      // Keep going forward until next danger detected
      if(dangerLeftDetected || dangerRightDetected){
        // Finished previous sequence, reset
        dangerLeftDetected = false;
        dangerRightDetected = false;
        dangerState = IDLE;
      } else {
        forward();
      }
      break;

    default:
      dangerState = IDLE;
      break;
  }
  
  delay(LOOP_DELAY);
}
