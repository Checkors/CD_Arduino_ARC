

#ifndef CROPDR_MTR_LIB_H
#define CROPDR_MTR_LIB_H
#include "Arduino.h"


void mtrPwmInit(void);

void SetDutyCycle_Right(int dutyCycle);

void SetDutyCycle_Left(int dutyCycle);


// Right Motor Control Functions
void RightMotor1_SetDutyCycle(unsigned int dutyCycle); // Front

void RightMotor2_SetDutyCycle(unsigned int dutyCycle); // Back

// Left Motor Conrol Functions
void LeftMotor1_SetDutyCycle(unsigned int dutyCycle); // Front

void LeftMotor2_SetDutyCycle(unsigned int dutyCycle); // Back


#endif