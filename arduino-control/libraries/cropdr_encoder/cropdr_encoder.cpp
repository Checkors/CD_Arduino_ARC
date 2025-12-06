
#include "cropdr_encoder.h"

#define _USE_MATH_DEFINES

#define ENC_TIMEOUT_COUNT 30000
#define EMA_ALPHA 0.03125

#define WHEEL_D_MM 774.7
#define WHEEL_GEAR_RATIO 24/21
#define ROT_PER_PULSE   0.00068627



#define ENC1_INTERRUPT Enc1_Handler
#define ENC1_PIN1 16  //A8, Encoder 1, pin 1, Ch. A from motor. Used for reading speed
#define ENC1_PIN2 17 //A9, Encoder 1, pin 2, Ch. B from motor. used for determining direction in combination with pin 1 (no interrupt)
#define ENC1_DIR_PORTNUM 1


#define ENC2_INTERRUPT Enc2_Handler
#define ENC2_PIN1 18  //A10, Encoder 2, pin 1, Ch. A from motor. Used for reading speed
#define ENC2_PIN2 19 //A11, Encoder 2, pin 2, Ch. B from motor. used for determining direction in combination with pin 1 (no interrupt)
#define ENC2_DIR_PORTNUM 3

#define ENC3_INTERRUPT Enc3_Handler
#define ENC3_PIN1 20  //A12, Encoder 3, pin 1, Ch. A from motor. Used for reading speed
#define ENC3_PIN2 21 //A13, Encoder 3, pin 2, Ch. B from motor. used for determining direction in combination with pin 1 (no interrupt)
#define ENC3_DIR_PORTNUM 5

#define ENC4_INTERRUPT Enc4_Handler
#define ENC4_PIN1 22  //A14, Encoder 4, pin 1, Ch. A from motor. Used for reading speed
#define ENC4_PIN2 23 //A15, Encoder 4, pin 2, Ch. B from motor. used for determining direction in combination with pin 1 (no interrupt)
#define ENC4_DIR_PORTNUM 7

#define ENC_PORT PINK // 




// RECEIVE PACKET
typedef struct {
  volatile unsigned long EMA;
  volatile unsigned long lastUp;
  volatile bool noSpin; 
  volatile bool dir;
  //uint8_t packetComplete; // Not  data, flag for constructing packet if interrupted.
} encoder;


static volatile encoder EncArray[4];

void Enc1_Handler(void) {
  unsigned long curTime = micros();
  EncArray[0].EMA = EncArray[0].EMA*(1-EMA_ALPHA) + EMA_ALPHA*(curTime - EncArray[0].lastUp);
  //EncArray[0].noSpin = (curTime - EncArray[0].lastUp > ENC_TIMEOUT_COUNT);
  EncArray[0].lastUp = curTime;
  EncArray[0].dir = (ENC_PORT & (1 << ENC1_DIR_PORTNUM)) != 0;
}

void Enc2_Handler(void) {
  unsigned long curTime = micros();
  EncArray[1].EMA = EncArray[1].EMA*(1-EMA_ALPHA) + EMA_ALPHA*(curTime - EncArray[1].lastUp);
  //EncArray[1].noSpin = (curTime - EncArray[1].lastUp > ENC_TIMEOUT_COUNT);
  EncArray[1].lastUp = curTime;
  EncArray[1].dir = (ENC_PORT & (1 << ENC2_DIR_PORTNUM)) != 0;
}

void Enc3_Handler(void) {
  unsigned long curTime = micros();
  EncArray[2].EMA = EncArray[2].EMA*(1-EMA_ALPHA) + EMA_ALPHA*(curTime - EncArray[2].lastUp);
  //EncArray[2].noSpin = (curTime - EncArray[2].lastUp > ENC_TIMEOUT_COUNT);
  EncArray[2].lastUp = curTime;
  EncArray[2].dir = (ENC_PORT & (1 << ENC3_DIR_PORTNUM)) != 0;
}

void Enc4_Handler(void) {
  unsigned long curTime = micros();
  EncArray[3].EMA = EncArray[3].EMA*(1-EMA_ALPHA) + EMA_ALPHA*(curTime - EncArray[3].lastUp);
  //EncArray[3].noSpin = (curTime - EncArray[3].lastUp > ENC_TIMEOUT_COUNT);
  EncArray[3].lastUp = curTime;
  EncArray[3].dir = (ENC_PORT & (1 << ENC4_DIR_PORTNUM)) != 0;
  
}

void Encoder_Init(void) {
  // put your setup code here, to run once: 
  EncArray[0].EMA = 0;
  EncArray[0].lastUp = 0;
  EncArray[0].noSpin = 0;
  EncArray[0].dir = 0;

  EncArray[1].EMA = 0;
  EncArray[1].lastUp = 0;
  EncArray[1].noSpin = 0;
  EncArray[1].dir = 0;

  EncArray[2].EMA = 0;
  EncArray[2].lastUp = 0;
  EncArray[2].noSpin = 0;
  EncArray[2].dir = 0;

  EncArray[3].EMA = 0;
  EncArray[3].lastUp = 0;
  EncArray[3].noSpin = 0;
  EncArray[3].dir = 0;

  pinMode(ENC1_PIN2, INPUT);
  pinMode(ENC1_PIN1, INPUT_PULLUP);

  pinMode(ENC2_PIN2, INPUT);
  pinMode(ENC2_PIN1, INPUT_PULLUP);

  pinMode(ENC3_PIN2, INPUT);
  pinMode(ENC3_PIN1, INPUT_PULLUP);

  pinMode(ENC4_PIN2, INPUT);
  pinMode(ENC4_PIN1, INPUT_PULLUP);

  attachPinChangeInterrupt(ENC1_PIN1, ENC1_INTERRUPT, RISING);
  enablePinChangeInterrupt(ENC1_PIN1);
  
  attachPinChangeInterrupt(ENC2_PIN1, ENC2_INTERRUPT, RISING);
  enablePinChangeInterrupt(ENC2_PIN1);
  
  attachPinChangeInterrupt(ENC3_PIN1, ENC3_INTERRUPT, RISING);
  enablePinChangeInterrupt(ENC3_PIN1);

  attachPinChangeInterrupt(ENC4_PIN1, ENC4_INTERRUPT, RISING);
  enablePinChangeInterrupt(ENC4_PIN1);
  
}

bool Enc_Spinng(uint8_t encNum){
if (micros() - EncArray[encNum].lastUp > ENC_TIMEOUT_COUNT){
  return 0;
} else {
  return 1;
}

}

int Enc_GetPeriod(uint8_t encNum) {
  // Uses an Exponential moving average to determine HZ
  if (Enc_Spinng(encNum)){
    return EncArray[encNum].EMA;
  } else {
    return ENC_TIMEOUT_COUNT;
  }
}

// -1 or 1 for direction, 0 for not spinning
int Enc_GetDir(uint8_t encNum){
  if (!Enc_Spinng(encNum)){
    return (-1 +2*EncArray[encNum].dir);
  } else{
    return 0;
  }

}


double Enc_GetSpeed(uint8_t encNum) {
  // Uses an Exponential moving average to determine HZ
    double Period = Enc_GetPeriod(encNum);
    if (Enc_Spinng(encNum)) {
      return (1000000/(Period))*ROT_PER_PULSE*WHEEL_GEAR_RATIO*WHEEL_D_MM*M_PI;
    } else {
      return 0;
      }
}
