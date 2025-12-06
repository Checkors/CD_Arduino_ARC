#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <u-blox_config_keys.h>
#include <u-blox_structs.h>



#define GPS_I2C_ADDRESS 0x42
#define TEST_PIN 38
#define TEST_PWM_PIN 12
#define TEST_PWM_DCREG OCR1B

SFE_UBLOX_GNSS myGPS;
//unsigned int TEST_PIN = 38;
//bool pinControl = 0;

uint8_t pinState = 0;
int i = 0;
uint8_t test_PWM_DC = 127;  // Value can be between 0 and 255.

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.print("SERIAL START\r\n");
  Wire.begin();

  //GPS Setup 
  if (!myGPS.begin()) {
    //Serial.print("GPS CONNECTION FAILED\r\n");
  } else {
    //Serial.print("GPS CONNECTION SUCCESSFUL, BEGINNING OPERATION LOOP\r\n");
  }
  // Test IO
  pinMode(TEST_PIN, OUTPUT);
  digitalWrite(TEST_PIN, pinState);

  // PWM Setup
  TCCR1A = 0;
  TCCR1B = 0;
  
  TCCR1A = _BV(WGM11) | _BV(COM1A1) | _BV(COM1B1);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12) | _BV(CS10);

  pinMode(TEST_PWM_PIN, OUTPUT);
  
  TEST_PWM_DCREG = test_PWM_DC;  // PWM DUTY CYCLE, 0 to 255, calculated by (OCRXX + 1) / 255
}

void loop() {
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    char receiveCon[64];
    sprintf(receiveCon, "RECEIVED: %c\r\n", inByte);
    Serial.print(receiveCon);
    switch (inByte) {
      case '+':
        Serial.print("PWM UP\r\n");
        test_PWM_DC = (test_PWM_DC + 1 == 255) ? 255 : test_PWM_DC + 1;
        //test_PWM_DC -= 100;
        sprintf(receiveCon, "PWM +Duty cycle: %i\r\n", TEST_PWM_DCREG);
        //TEST_PWM_DCREG = test_PWM_DC;
        analogWrite(TEST_PWM_PIN, test_PWM_DC);
        Serial.print(receiveCon);
        break;

      case '-':
        Serial.print("PWM DOWN\r\n");
        test_PWM_DC = (test_PWM_DC - 1 == 0) ? 0 : test_PWM_DC - 1;
        //test_PWM_DC += 100;
        sprintf(receiveCon, "PWM -Duty cycle: %i\r\n", TEST_PWM_DCREG);
        analogWrite(TEST_PWM_PIN, test_PWM_DC);
        //TEST_PWM_DCREG = test_PWM_DC;
        Serial.print(receiveCon);
        break;

      case 'd':
        Serial.print("PINCHANGE\r\n");
        pinState = 1 - pinState;

        sprintf(receiveCon, "TEST PIN/MOTOR DIR: %i\r\n", pinState);
        Serial.print(receiveCon);
        digitalWrite(TEST_PIN, pinState);
        break;
      case 'G':
        if (myGPS.getPVT()) {
          long lati = myGPS.getLatitude();
          long longi = myGPS.getLongitude();
          long alti = myGPS.getAltitude();
          sprintf(receiveCon, "(G)PS: %li,%li,%li\r\n", lati, longi, alti);
          Serial.print(receiveCon);
        }
        break; 
      case 'L':
        Serial.print("GPS DATA GATHERING 3 MINUTES");
        for (i = 0; i < 360; i++) {
          if (myGPS.getPVT()) {
            long lati = myGPS.getLatitude();
            long longi = myGPS.getLongitude();
            long alti = myGPS.getAltitude();
            sprintf(receiveCon, "%li,%li,%li\r\n", lati, longi, alti);
            Serial.print(receiveCon);
          }
          delay(500);
        }
        break;
      case 'E':
        Serial.print("I've had (E)nough OF THIS SHIT BLYAT\r\n");
        break;

      case 'P':
        pinState ^= 0x01;
        sprintf(receiveCon, "(P)IN TEST, PIN:%i, VAL:%i\r\n", TEST_PIN, pinState);
        Serial.print(receiveCon);
        //Serial.print("FUCKIN PRINT ME\r\n");
        digitalWrite(TEST_PIN, pinState);
        break;

      case 'C':
        if (!myGPS.getInvalidLlh()) {
          Serial.print("(C)HECK: VALID LLH\r\n");
        } else {
          Serial.print("(C)HECK: INVALID LLH\r\n");
        }
        break;
      case 'F':
        int fixType = myGPS.getFixType();
        sprintf(receiveCon, "(F)ix TYPE: %i\r\n", fixType);
        Serial.print(receiveCon);
        break;
        // END OF SWITCH
    }
    // END OF IF
  }
  // END OF LOOP
}
