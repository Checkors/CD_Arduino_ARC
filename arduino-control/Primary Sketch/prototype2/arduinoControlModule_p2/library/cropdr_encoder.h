// ENCODER LIBRARY


#ifndef CROPDR_ENC_LIB_H
#define CROPDR_ENC_LIB_H
#include "Arduino.h"
#include <PinChangeInterrupt.h>








void Encoder_Init(void);

int Enc1p1_GetHz(void);

int Enc1_GetDir(void);

void Encoder1_Pin1_Handler(void);
#endif