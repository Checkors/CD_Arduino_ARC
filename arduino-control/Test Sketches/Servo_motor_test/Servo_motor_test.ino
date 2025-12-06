#include <Wire.h>
#include <Stepper.h>

// STEPPER IS SM-42BYG011-25
#define STEPPER_A 40  // A+
#define STEPPER_B 41  // B+
#define STEPPER_C 42  // A-
#define STEPPER_D 43  // B-

static char stepState = 'W';
static uint8_t pause = 1;
unsigned long timeCount = 0;

//Stepper anson_Stepper(200, STEPPER_A, STEPPER_B, STEPPER_C, STEPPER_D);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.print("SERIAL START, SEND ANY CHAR TO START/PAUSE\r\n");

  pinMode(STEPPER_A, OUTPUT);
  pinMode(STEPPER_B, OUTPUT);
  pinMode(STEPPER_C, OUTPUT);
  pinMode(STEPPER_D, OUTPUT);

  digitalWrite(STEPPER_A, 0);
  digitalWrite(STEPPER_B, 0);
  digitalWrite(STEPPER_C, 0);
  digitalWrite(STEPPER_D, 0);
  // anson_Stepper.setSpeed(60);
}



void loop() {
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    char receiveCon[64];
    //sprintf(receiveCon, "R: %c, STATE: %c\r\n", inByte, stepState);
    if (pause) {
      Serial.print("STARTING\r\n");
      //anson_Stepper.setSpeed(60);
    } else {
      Serial.print("PAUSING\r\n");
      //anson_Stepper.setSpeed(0);
    }
    pause = 1 - pause;
  }

  if ((!pause) && (millis() - timeCount > 100)) {
    Serial.print("STEP\r\n");
    step();
    timeCount = millis();
  }


  // put your main code here, to run repeatedly:
}

// Stepper motor state machine loop.
// W = 45 degrees
// X = 135 degrees
// Y = 225 degrees
// Z = 315 degrees
void step(void) {

  switch (stepState) {
    case 'W':

      digitalWrite(STEPPER_A, 1);
      digitalWrite(STEPPER_B, 1);
      digitalWrite(STEPPER_C, 0);
      digitalWrite(STEPPER_D, 0);

      stepState = 'X';
      break;
    case 'X':

      digitalWrite(STEPPER_A, 0);
      digitalWrite(STEPPER_B, 1);
      digitalWrite(STEPPER_C, 1);
      digitalWrite(STEPPER_D, 0);

      stepState = 'Y';
      break;
    case 'Y':

      digitalWrite(STEPPER_A, 0);
      digitalWrite(STEPPER_B, 0);
      digitalWrite(STEPPER_C, 1);
      digitalWrite(STEPPER_D, 1);

      stepState = 'Z';
      break;
    case 'Z':

      digitalWrite(STEPPER_A, 1);
      digitalWrite(STEPPER_B, 0);
      digitalWrite(STEPPER_C, 0);
      digitalWrite(STEPPER_D, 1);

      stepState = 'W';
      break;
    default:
      stepState = 'W';
      break;
  }
}
