                                                       Obstacle-Detecting Robot Car
                                                                 By Koray
![image alt](https://github.com/koray9012/Koray-s-obstacle-detecting-car/blob/a4357bc2f437e83db109d0d0175d221761f2d390/20260106_220205.jpg )                                                                
A fully autonomous robot which detects the end of the surface its on and stops before falling off

Key Features:

• Works on 2 9V batteries

• fully autonomous

• Does a specific maneuver based on which sensor detects first 

How to use: 

To use it you need to put it on a flat surface and get your 2 9V batteries. The first one you will connect to the barrel jack of the arduino with a battery clip and the other one you will connect to the screws 12V and gnd of the L298N driver and then after that it turns on and starts moving. I recomment firstly connecting the driver battery so you have time to put the batteries in the car so they dont fall off and slow down or tangle in the car.

Why i made it: 

Before finding about Hackclub i was mesmerised by autonomous cars like Tesla so i started researching on how they look. After i found out about Hackclub and Blueprint i decided that i wanted to try it and learn something new and maybe win a reward so when choosing an idea i found out that i can use ultrasonic sensors to measure distances so i decided to make an autonomous car which measures the distance from the ground where if it reaches over 50cm it will detect a fall and stop.

Wiring:

![image alt](https://github.com/user-attachments/assets/81c18ad3-d5b4-4e3d-8700-cb26b50db962)

code:

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

Bill of materials: 

          Item               ⏐ Quantity ⏐ Price (USD) ⏐ link

Arduino UNO R3           ⏐    1     ⏐   7.57 USD  ⏐ https://elimex.bg/product/71201-kit-k2014-razvoyna-platka-s-atmega328p-smd-usb-b

L298N motor driver       ⏐    1     ⏐   4.63 USD  ⏐ https://elimex.bg/product/71197-kit-k2010-drayver-za-postoyannotokovi-motori

HC-SR04 Sensor           ⏐    2     ⏐   2.28 USD x2 = 4.56 USD  ⏐ https://elimex.bg/product/71196-kit-k2009-hc-sr04-ultrazvukov-priemo-predavatel
            
Project DC motors        ⏐    2     ⏐   2.28 USD x2 = 4.56 USD  ⏐ https://elimex.bg/product/79622-kit-k2178-postoyannotokov-motor-za-robo-platforma

Jumper cables            ⏐    14    ⏐ 2.98 USD + 2.30 USD = 6.28 USD ⏐ https://elimex.bg/product/75823-komplekt-provodnitsi-40-broya-s-konektori-mazhki-zhenski-30sm AND                                                                  https://elimex.bg/product/74894-komplekt-provodnitsi-40-broya-s-konektori-mazhki-mazhki-20sm
    
9V battery               ⏐    2     ⏐   2.28 USD x2 = 4.56 USD ⏐ https://elimex.bg/product/93070-bateriq-6lp3146-varta-energy

Battery clip (jack)      ⏐    1     ⏐   0.53 USD  ⏐ https://elimex.bg/product/77718-f172b-battery-clip-9v-to-dc-plug-2-1x5-5mm

Battery clip (no jack)   ⏐    1     ⏐   0.23 USD  ⏐ https://elimex.bg/product/14153-f172a-battery-clip-9v

Body of the robot        ⏐    1     ⏐   15.28 USD ⏐ https://elimex.bg/product/74873-kit-k2122-kit01-robo-platforma-s-2-kolela-s-2-dc-motora2-osnovi
 
