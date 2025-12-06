// ENCODER LIBRARY


#ifndef CROPDR_ENC_LIB_H
#define CROPDR_ENC_LIB_H
#include "Arduino.h"
#include <PinChangeInterrupt.h>
#include  <math.h>

void Encoder_Init(void);

double Enc_GetSpeed(uint8_t);

int Enc_GetPeriod(uint8_t);

int Enc_GetDir(uint8_t);

#endif