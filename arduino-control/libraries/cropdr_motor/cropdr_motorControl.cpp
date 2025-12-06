/*

Timer 1A: D11
Timer 1B: D12  

Timer 3A: D5
Timer 3B: D2
Timer 3C: D3

Timer 4A: D6
Timer 4B: D7
Timer 4C: D8

Timer 5A: D46
Timer 5B: D45
Timer 5C: D44

*/
#include "cropdr_motorControl.h"

#define DIR_PORT PORTA // Port For controlling Direction

#define R_MTR1_DIR 0 // pin 22
#define R_MTR2_DIR 1 // pin 23
#define R_DIR_BITS 0x03

#define L_MTR1_DIR 2 // pin 24
#define L_MTR2_DIR 3 // pin 25
#define L_DIR_BITS 0x0C

// RIGHT MOTORS //
// PWM pins for Right motors
#define R_MTR1_PWM 12 // B  //PWM Pins for the output of the signal // FRONT RIGHT MOTOR CONTROL
#define R_MTR2_PWM 11 // A  // BACK RIGHT MOTOR CONTROL

// Duty cycle registers for Right motors
#define R_MTR1_DCREG OCR1B
#define R_MTR2_DCREG OCR1A

// Right motors, Timer 1 Control registers
#define R_CR_A TCCR1A   // Timer 1 output mode
#define R_CR_B TCCR1B   // Input Compare, Wavegenerator, Timer 1 Prescaler.

// LEFT MOTORS //
// PWM pins for Left Motors
#define L_MTR1_PWM 5  // A // FRONT LEFT MOTOR
#define L_MTR2_PWM 2  // B // BACK LEFT MOTOR



// Duty Cycle registers for Left Motors
#define L_MTR1_DCREG OCR3A
#define L_MTR2_DCREG OCR3B

// Left motors, Timer 2 Control registers
#define L_CR_A TCCR3A
#define L_CR_B TCCR3B



/* Initialization of PWMs for motor control, fixed frequency relevant to motors.
*/
void mtrPwmInit(void){

// Direction GPIO

 // DDRA |= 0x0F;
  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);

  pinMode(24, OUTPUT);
  pinMode(25, OUTPUT);


// Right motors setup
  pinMode(R_MTR1_PWM, OUTPUT); // Setting PWM outputs
  pinMode(R_MTR2_PWM, OUTPUT);
  R_CR_A = 0;
  R_CR_B = 0;
  
  R_CR_A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
  R_CR_B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); 

  ICR1 = 1599; // TOP VALUE

  R_MTR1_DCREG = 0; 
  R_MTR2_DCREG = 0;

// Left motors setup
  pinMode(L_MTR1_PWM, OUTPUT);
  pinMode(L_MTR2_PWM, OUTPUT);
  L_CR_A = 0;
  L_CR_B = 0;

  L_CR_A = _BV(COM3A1) | _BV(COM3B1) | _BV(WGM31); // VOM2A1:PIN 10
  L_CR_B = _BV(WGM33) | _BV(WGM32) | _BV(CS30); // Frequency scaler

  ICR3 = 1599; // TOP VALUE
  
  L_MTR1_DCREG = 0;
  L_MTR2_DCREG = 0;
}



// SIMPLE CONTROL FUNCTIONS

/* PORT_SetDirection(uint8_t dir)
dir of e
Sets the direction. 0 for forward, 1 for reverse;
*/
void PORT_SetDirection(uint8_t dirBits){
  DIR_PORT |= (dirBits&0x0F);
}

/* Right_SetDirection(uint8_t dir)
Sets the direction. 0 for forward, 1 for reverse;
*/
void Right_SetDirection(uint8_t dir){ 
  dir = dir > 1 ? 1 : dir;
  PORT_SetDirection(R_DIR_BITS*dir);
}


void Left_SetDirection(uint8_t dir){ 
  dir = dir > 1 ? 1 : dir;
  PORT_SetDirection(L_DIR_BITS*dir);
}


/* void RightMotor1_SetDutyCycle(unsigned int Dutycycle)
Inputs:
- unsigned int dutyCycle // value from 0 to 255
Sets the duty Cycle for the Front Right Motor */
void RightMotor1_SetDutyCycle(unsigned int dutyCycle){
  if (dutyCycle > 1599){
    return;
  }
  R_MTR1_DCREG = dutyCycle;
  return;
}

/* void RightMotor2_SetDutyCycle(unsigned int Dutycycle)
Inputs:
- unsigned int dutyCycle // value from 0 to 255
Sets the duty Cycle for the Back Right Motor */
void RightMotor2_SetDutyCycle(unsigned int dutyCycle){

  if (dutyCycle > 1599){
    return;
  }
  R_MTR2_DCREG = dutyCycle;
  return;
}

/* void LeftMotor1_SetDutyCycle(unsigned int Dutycycle)
Inputs:
- unsigned int dutyCycle // value from 0 to 255
Sets the duty Cycle for the Front Left Motor */
void LeftMotor1_SetDutyCycle(unsigned int dutyCycle){
  if (dutyCycle > 1599){
    return;
  }
  L_MTR1_DCREG = dutyCycle;
  return;
}

/* void LeftMotor2_SetDutyCycle(unsigned int Dutycycle)
Inputs:
- unsigned int dutyCycle // value from 0 to 255
Sets the duty Cycle for the Back Left Motor */
void LeftMotor2_SetDutyCycle(unsigned int dutyCycle){
  if (dutyCycle > 1599){
    return;
  }
  L_MTR2_DCREG = dutyCycle;
  return;
}


// 

/* void Left_SetDutyCycle(unsigned int Dutycycle)
Sets the duty cycle for both left side Motors*/
void SetDutyCycle_Right(int dutyCycle){
  if (abs(dutyCycle) > 1599){
    return;
  }
  if (dutyCycle < 0){
    dutyCycle *= -1;
    Right_SetDirection(1);
  } 
  RightMotor1_SetDutyCycle(dutyCycle);
  RightMotor2_SetDutyCycle(dutyCycle);
  return;
}


/* void Left_SetDutyCycle(unsigned int Dutycycle)
Sets the duty cycle for both left side Motors*/
void SetDutyCycle_Left(int dutyCycle){
  if (abs(dutyCycle) > 1599){
    return;
  }
  if (dutyCycle < 0){
    dutyCycle *= -1;
    Left_SetDirection(1);
  } 
  LeftMotor1_SetDutyCycle(dutyCycle);
  LeftMotor2_SetDutyCycle(dutyCycle);
  return;
}


