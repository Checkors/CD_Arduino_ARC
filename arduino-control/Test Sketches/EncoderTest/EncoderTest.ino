
#include "PinChangeInterrupt.h"

#define EMA_ALPHA 0.03125


#define ENC1_PIN1_FUNCTION Encoder1_Pin1_Handler
#define ENC1_PIN1 16  //A8, Encoder 1, pin 1, Ch. A from motor. Used for reading speed
#define ENC1_PIN2 17 //A9, Encoder 1, pin 2, Ch. B from motor. used for determining direction in combination with pin 1 (no interrupt)
#define ENC_PORT PORTK
#define ENC1_PIN2_PORT 1


volatile unsigned long enc1_p1_EMA;
volatile unsigned long enc_p1_lastUp;
volatile bool enc1_noSpin; 
volatile bool enc1_dir1;


char testString[64];

void setup() {
  // put your setup code here, to run once:
  enc1_noSpin = 0;
  enc1_p1_EMA = 0;
  enc_p1_lastUp = 0;
  enc1_dir1 = 0;

  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.print("SERIAL START\r\n");
  Serial.print("ENCODER NOW READING\r\n");

  pinMode(ENC1_PIN2, INPUT);
  pinMode(ENC1_PIN1, INPUT_PULLUP);
  attachPinChangeInterrupt(ENC1_PIN1, ENC1_PIN1_FUNCTION, RISING);
  enablePinChangeInterrupt(ENC1_PIN1);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  sprintf(testString, "FUNC OUT: %li, DIR: %i\r\n", Enc1p1_GetHz(), Enc1_GetDir());
  Serial.print(testString);
  
  delay(250);
}

unsigned long Enc1p1_GetHz(void) {
  // Uses an Exponential moving average to determine HZ
  if ((micros() - enc_p1_lastUp > 1000000) || (enc_p1_lastUp == 0)) {  // Raises a flag if the time is greater than a second.
    enc1_noSpin = 1;
    return 0;
  }
  else{
    enc1_noSpin = 0;
    return enc1_p1_EMA;
  }
}


// 0 or 1 for direction, -1 for not spinning
int Enc1_GetDir(void){
  if (enc1_noSpin){
    return -1;
  } else{
    return enc1_dir1;
  }

}



void Encoder1_Pin1_Handler(void) {
  unsigned long curTime = micros();
  
  
  enc1_p1_EMA = enc1_p1_EMA*(1-EMA_ALPHA) + EMA_ALPHA*(curTime - enc_p1_lastUp);
  enc_p1_lastUp = curTime;
  enc1_dir1 = (ENC_PORT & (1 << ENC1_PIN2_PORT)) != 0;
  
  
}
