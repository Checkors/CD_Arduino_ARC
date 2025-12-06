/* OPERATION DESCRIPTION
This is the command and control sketch for running specific set code on the ARduino mega from another micro-controller.



*/

// LIBRARY INCLUSIONS

#include <Wire.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <u-blox_config_keys.h>
#include <u-blox_structs.h>
#include "PacketSerial.h"
#include <cropdr_encoder.h>
#include <cropdr_motorControl.h>
#include "TimeLib.h"
#include "cropdr_PI.h"

#define PINGTEST 1

// MACRO #DEFINE DECLARATION
#define BUFFER_SIZE 8
#define SEND_PACKET_SIZE 37
#define GPS_I2C_ADDRESS 0x42
#define UPDATE_TIME 100  

// GLOBAL VARIABLE DECLARATION
PacketSerial serialCon;
SFE_UBLOX_GNSS myGPS;

// TYPEDEFS


// RECEIVE PACKET
typedef struct {
  int setLeftTarget;    //0
  int setRightTarget;   //1
  char placeFlagActive;  //2
  unsigned long Epoch;   //3
  unsigned long Millis;  //4
  //uint8_t packetComplete; // Not  data, flag for constructing packet if interrupted.
} receivePacket;


// SEND PACKET
typedef struct {
  int activeLeftTarget;       //0
  int activeRightTarget;      //1
  int leftSpeed;               //2
  int rightSpeed;              //3
  uint8_t gpsLLHValid;         //4
  uint8_t gpsFixType;          //5
  unsigned long gpsLongitude;  //6
  unsigned long gpsLatitude;   //7
  unsigned long gpsEpoch;      //6
  unsigned long gpsMillis;     //7
  char shooterState;           //8
  unsigned long latency;       //9
                               // Stored in memory.
} sendPacket;


// CIRCULAR STACK (WRITE FIRST, THEN INCREMENT)
typedef struct {
  receivePacket circBuffer[BUFFER_SIZE];
  uint8_t index;
} RxCircularStack;



// GLOBAL STRUCTS
volatile RxCircularStack RXbuffer;
volatile sendPacket globVars;
static unsigned long testTimer = 0;
static char testString[128];


/*/////////////////////////////////////////////////////////////////////////////////////
      OPERATION FUNCTIONS ////////////////////////////////////////////////////////
*/
/////////////////////////////////////////////////////////////////////////////////////


uint8_t globVarUpdate(void) {
  
  //LOCAL UPDATES
  globVars.leftSpeed = Enc_GetPeriod(0) * Enc_GetDir(0);

  if (myGPS.isConnected()){
    globVars.gpsEpoch = myGPS.getUnixEpoch();
    setTime(globVars.gpsEpoch);
  }
  
  //DICTATED UPDATES
  receivePacket update = callLastBroadcast();

  globVars.activeLeftTarget = update.setLeftTarget;
  globVars.activeRightTarget = update.setRightTarget;

  Motor_PI_SetRight(globVars.activeRightTarget);
  Motor_PI_SetLeft(globVars.activeLeftTarget);
  

  

  return 0;
}



/*/////////////////////////////////////////////////////////////////////////////////////
      HELPER FUNCTIONS ////////////////////////////////////////////////////////////////
*/
/////////////////////////////////////////////////////////////////////////////////////

/* Send Debug
takes a character point (string or character array) and sends an encoded version over the serial connection.
*/
void sendDebug(char* message) {
  size_t debugSize = strlen(message);
  serialCon.send(message, debugSize);
  return;
}




// TX num packets enqued

// RX num packets enqued.

/*/////////////////////////////////////////////////////////////////////////////////////
      SERIAL CONTROL FUNCTIONS ////////////////////////////////////////////////////////
*/
/////////////////////////////////////////////////////////////////////////////////////

// INCOMING COMMAND PACKET
void RXpacketPush(const uint8_t* buffer, size_t size) {


  if (size != 11) {
    sendDebug("FAILED_SIZE_TEST\r\n");
    return;
  }
  // WRITE FIRST, MOVE NEXT
  // DEFINING DATA OF PROCESS PACKET

  uint8_t timeArray[4];
  uint8_t intConvert[2];

  intConvert[0] = buffer[0];
  intConvert[1] = buffer[1];
  memcpy(&RXbuffer.circBuffer[RXbuffer.index].setLeftTarget, intConvert, sizeof(int));

  intConvert[0] = buffer[2];
  intConvert[1] = buffer[3];
  memcpy(&RXbuffer.circBuffer[RXbuffer.index].setRightTarget, intConvert, sizeof(int));
  

  RXbuffer.circBuffer[RXbuffer.index].placeFlagActive = buffer[4];

  

  timeArray[0] = buffer[5];
  timeArray[1] = buffer[6];
  timeArray[2] = buffer[7];
  timeArray[3] = buffer[8];

  memcpy(&RXbuffer.circBuffer[RXbuffer.index].Epoch, timeArray, sizeof(long));

  intConvert[0] = buffer[9];
  intConvert[1] = buffer[10];

  memcpy(&RXbuffer.circBuffer[RXbuffer.index].Millis, intConvert, sizeof(int));

  RXbuffer.index = (RXbuffer.index + 1)%BUFFER_SIZE;
  return;
}

/* callLastBroadcast(void)
Returns the last sent broadcast.
*/
receivePacket callLastBroadcast(void) {
  static receivePacket lastBroadcast;
  memcpy(&lastBroadcast, &RXbuffer.circBuffer[((RXbuffer.index + BUFFER_SIZE) - 1)%BUFFER_SIZE], sizeof(receivePacket));
  return lastBroadcast;
}


//TX SEND
uint8_t TXpacketSend(void) {
  static uint8_t sendBuffer[SEND_PACKET_SIZE];
  int i = 0;

  uint8_t longConv[4];
  uint8_t intConv[2];

  for (i = 0; i < SEND_PACKET_SIZE; i++) {

    switch (i) {
      case 2: // DATA BREAKS FOR PARSING ON PYTHON
      case 5:
      case 8:
      case 11:
      case 13:
      case 15:
      case 20:
      case 25:
      case 30:
      case 35:
      case 37:
      case 42:
        sendBuffer[i] = ':';
        break;
      
      // LEFT PWM, INT (2), ind 0
      case 0: 
        memcpy(intConv, &globVars.activeLeftTarget, sizeof(int));
      case 1:
        sendBuffer[i] = intConv[i];
        break;
      
      // RIGHT PWM, INT (2), ind 1
      case 3: 
        memcpy(intConv, &globVars.activeRightTarget, sizeof(int));
      case 4:
        sendBuffer[i] = intConv[i-3];
        break;
        
      // LEFT ENCODER, INT (2), ind 2 /***********************************************************************************/
      case 6: 
        memcpy(intConv, &globVars.leftSpeed, sizeof(int));
      case 7: 
        sendBuffer[i] = intConv[i - 6];
        break;
    
      // RIGHT ENCODER, INT (2), ind 3 /***********************************************************************************/
      case 9: 
        memcpy(intConv, &globVars.rightSpeed, sizeof(int));
      case 10: 
        sendBuffer[i] = intConv[i - 9];
        break;
      
      // GPS LLH VALID, UINT_8 (1), ind 4         /***********************************************************************************/
      case 12: 
        sendBuffer[i] = globVars.gpsLLHValid;
        break;

      // GPS FIX TYPE, UINT_8 (1), ind 5
      case 14: 
        sendBuffer[i] = globVars.gpsFixType;
        break;

      // GPS LATITUDE, LONG INT (4), ind 6
      case 16: 
        memcpy(longConv, &globVars.gpsLatitude, sizeof(long));
      case 17:
      case 18:
      case 19: 
        sendBuffer[i] = longConv[i - 16];
        break;
      
      // GPS LONGITUDE, LONG INT (4), ind 7
      case 21: 
        memcpy(longConv, &globVars.gpsLongitude, sizeof(long));
      case 22:
      case 23:
      case 24:
        sendBuffer[i] = longConv[i - 21];
        break;

      // GPS UNIX EPOCH TIME, LONG INT (4), ind 8
      case 26:
        memcpy(longConv, &globVars.gpsEpoch, sizeof(long));
      case 27:
      case 28:
      case 29:
        sendBuffer[i] = longConv[i - 26];
        break;

      // ARDUINO MILLIS FREE RUNNING TIME, LONG INT (4), ind 9
      case 31:
        memcpy(longConv, &globVars.gpsMillis, sizeof(long));
      case 32:
      case 33:
      case 34:
        sendBuffer[i] = longConv[i - 31];
        break;
      
      // FLAG SHOOTER STATE, CHAR (1), in 10
      case 36:
        sendBuffer[i] = uint8_t(globVars.shooterState);
         break;

      // ARDUINO LATENCY
      case 38:
        memcpy(longConv, &globVars.gpsMillis, sizeof(long));
      case 39:
      case 40:
      case 41:
        sendBuffer[i] = longConv[i - 38];
        break;
      case 43:
        sendBuffer[i] = '\r';
        break;
      case 44:
        sendBuffer[i] = '\n';
        break;
    }
  }
  serialCon.send(sendBuffer, SEND_PACKET_SIZE);
  return 0;
}




void setup() {
  // put your setup code here, to run once:
  serialCon.begin(115200);
  serialCon.setPacketHandler(&RXpacketPush);

  while(RXbuffer.index == 0){
    serialCon.update();
    delay(5);
  }
  setTime(callLastBroadcast().Epoch);

  sendDebug("SERIAL INITIATED\r\n");


  //MOTOR SETUP
  Motor_PI_Init();
  sendDebug("MOTOR PWM INITIATED");
  // 
  Encoder_Init();
  sendDebug("ENCODER INITIATED\r\n");


  //GPS Setup
  Wire.begin();
  if (!myGPS.begin()) {
    ;
    sendDebug("GPS CONNECTION FAILED\r\n");
    setTime(callLastBroadcast().Epoch);
    //Serial.print("GPS CONNECTION FAILED\r\n");
  } else {
    sendDebug("GPS INIT\r\n");
    myGPS.setAutoPVT(1);
  }

  sendDebug("SYSTEM START\r\n");

  testTimer = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  serialCon.update();  // CALL THIS FUNCTION ONLY ONCE PER LOOP
#if PINGTEST
  if (millis() - testTimer > UPDATE_TIME) {
    testTimer = millis();
    
    //sprintf(testString, "ENCODER LIB VAL: %i \r\n", Enc_GetPeriod(0) * Enc_GetDir(0));
    //sendDebug(testString);
    //sendDebug("UPDATE PACKET\r\n");
    Motor_PI_Update();
    sendDebug("\r\n");
    globVarUpdate();
    Motor_PI_Update();
    TXpacketSend();
  }
#endif
}
