#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <u-blox_config_keys.h>
#include <u-blox_structs.h>


int i;
#define GPS_I2C_ADDRESS 0x42

SFE_UBLOX_GNSS myGPS;
//unsigned int TEST_PIN = 38;
//bool pinControl = 0;
#define TEST_PIN 38
uint8_t pinState = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.print("SERIAL START\r\n");
  Wire.begin();

  if (!myGPS.begin()) {
    Serial.print("GPS CONNECTION FAILED\r\n");
  } else  { 
    Serial.print("GPS CONNECTION SUCCESSFUL, BEGINNING OPERATION LOOP\r\n");
  }

  pinMode(TEST_PIN, OUTPUT);
  digitalWrite(TEST_PIN, pinState);
}

void loop() {
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    char receiveCon[64];
    sprintf(receiveCon, "RECEIVED: %c\r\n", inByte);
    Serial.print(receiveCon);
    switch (inByte) {
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
        Serial.print("I've had (E)nough\r\n");
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
    }
  }



  // put your main code here, to run repeatedly:
}
