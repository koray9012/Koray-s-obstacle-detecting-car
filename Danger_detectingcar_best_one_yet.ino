// ---------- PINS ----------
#define TRIG_L 5
#define ECHO_L 4
#define TRIG_R 6
#define ECHO_R 7

#define IN1 11
#define IN2 10
#define IN3 9
#define IN4 8

// ---------- SETTINGS ----------
#define DANGER_MIN 55        // cm
#define WAIT_TIME 2000
#define BACKWARD_TIME 500
#define TURN90_TIME 400
#define TURN180_TIME 800
#define LOOP_DELAY 5         // faster loop

#define DANGER_CONFIRM 2     // faster detection

// ---------- STATE ----------
enum State { FORWARD, STOP1, BACKWARD, STOP2, TURN, STOP3 };
State state = FORWARD;

unsigned long stateTime = 0;

bool dangerLeft = false;
bool dangerRight = false;

int dangerCountL = 0;
int dangerCountR = 0;

// ---------- FUNCTIONS ----------
long readDistanceFast(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(1);
  digitalWrite(trig, HIGH);
  delayMicroseconds(8);      // shorter trigger pulse
  digitalWrite(trig, LOW);

  // MUCH shorter timeout → faster reaction
  long duration = pulseIn(echo, HIGH, 8000); // ~140 cm max

  if (duration == 0) return -1;

  return duration * 0.034 / 2;
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
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

  stopMotors();
}

// ---------- LOOP ----------
void loop() {
  unsigned long now = millis();

  long dL = readDistanceFast(TRIG_L, ECHO_L);
  long dR = readDistanceFast(TRIG_R, ECHO_R);

  // ---- FAST SENSOR LOGIC ----
  if (dL >= DANGER_MIN && dL != -1) dangerCountL++;
  else dangerCountL = 0;

  if (dR >= DANGER_MIN && dR != -1) dangerCountR++;
  else dangerCountR = 0;

  bool holeL = dangerCountL >= DANGER_CONFIRM;
  bool holeR = dangerCountR >= DANGER_CONFIRM;

  switch (state) {

    case FORWARD:
      forward();
      if (holeL || holeR) {
        dangerLeft = holeL;
        dangerRight = holeR;
        stopMotors();
        state = STOP1;
        stateTime = now;
      }
      break;

    case STOP1:
      if (now - stateTime >= WAIT_TIME) {
        backward();
        state = BACKWARD;
        stateTime = now;
      }
      break;

    case BACKWARD:
      if (now - stateTime >= BACKWARD_TIME) {
        stopMotors();
        state = STOP2;
        stateTime = now;
      }
      break;

    case STOP2:
      if (now - stateTime >= WAIT_TIME) {
        state = TURN;
        stateTime = now;
      }
      break;

    case TURN:
      if (dangerLeft && dangerRight) {
        turnLeft();
        if (now - stateTime >= TURN180_TIME) {
          stopMotors();
          state = STOP3;
          stateTime = now;
        }
      } else if (dangerLeft) {
        turnRight();
        if (now - stateTime >= TURN90_TIME) {
          stopMotors();
          state = STOP3;
          stateTime = now;
        }
      } else if (dangerRight) {
        turnLeft();
        if (now - stateTime >= TURN90_TIME) {
          stopMotors();
          state = STOP3;
          stateTime = now;
        }
      }
      break;

    case STOP3:
      if (now - stateTime >= WAIT_TIME) {
        dangerLeft = false;
        dangerRight = false;
        dangerCountL = 0;
        dangerCountR = 0;
        state = FORWARD;
      }
      break;
  }

  delay(LOOP_DELAY);
}