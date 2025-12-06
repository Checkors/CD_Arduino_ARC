#ifndef CROPDR_PI_H
#define CROPDR_PI_H

#include "Arduino.h"
#include "PID_v1.h"
#include "cropdr_motorControl.h"
#include "cropdr_encoder.h"


void Motor_PI_Init(void);

void Motor_PI_Update(void);

void Motor_PI_SetRight(int target);

void Motor_PI_SetLeft(int target);





#endif