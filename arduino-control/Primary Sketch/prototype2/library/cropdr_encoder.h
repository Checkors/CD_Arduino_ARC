// ENCODER LIBRARY

#ifndef CROPDR_ENCODER
#define CROPDR_ENCODER

#include "PinChangeInterrupt.h"

#if (ARDUINO >= 100)
#include "Arduino.h"
#endif
#endif

#define EMA_ALPHA 0.03125


#define ENC1_PIN1_FUNCTION Encoder1_Pin1_Handler
#define ENC1_PIN1 16  //A8, Encoder 1, pin 1, Ch. A from motor. Used for reading speed
#define ENC1_PIN2 17 //A9, Encoder 1, pin 2, Ch. B from motor. used for determining direction in combination with pin 1 (no interrupt)
#define ENC_PORT PORTK
#define ENC1_PIN2_PORT 1





void Encoder_Init(void)

unsigned long Enc1p1_GetHz(void) 

int Enc1_GetDir(void)

void Encoder1_Pin1_Handler(void) 








