/* OPERATION DESCRIPTION
This is the command and control sketch for running specific set code on the ARduino mega from another micro-controller.
Think of this as a more advanced version of the serial_test sketch that was presented previous, however instead of reading characters,
this sketch reads packets. These packets are encoded in the COBS standard, and need to be decoded by a COBS library before being properly read.
The data contained in these packets have a substructure to communicate necessary information about the packet being sent.
Packet data is expected to containin first, a byte representing process type, and a data type byte second,
leaving 126 Bytes for actual data.

This data substructure configuration I call a "processing packet".
The process type byte is there to define what this sketch should do with the data provided.
there are three types of processes
- Query         (Read)      [Q]
- Command       (Write)     [C]
- Synch         (Ping-Pong)  [S]
- Confirmation  (ACK)       [R]

// Query (Q), Command (C), Confirmation (R)
// long (l), int (i), char(c)

Query:
Query packets are there to draw data from one source to the other. If the arduino receives a query, it will go through a switch statement
that will output the requested data and return it.

Command:
Command packets are there write or change some operation on the arduino. If the arduino receives a command packet, it will go through a switch statement
to change the desired value or execute a specific function on the arduino.

Confirmation:
Confirmation packets are there to confirm data receipt and success or failure cases, there to add descriptions.

THIS CODE MAY HAVE MEMORY LEAKS, BE WARY WHEN ALLOCATING AND DE-ALLOCATING


*/

// LIBRARY INCLUSIONS
#include "PacketSerial.h"
#include <TimeLib.h>


#define PINGTEST 1

// MACRO #DEFINE DECLARATION
#define BUFFER_SIZE 8

// GLOBAL VARIABLE DECLARATION
PacketSerial serialCon;


// TYPEDEFS


// PROCESSING PACKET
typedef struct {
  uint8_t packetType;
  uint8_t dataType;
  size_t dataSize;  // Size of Data
  void *data;       // Stored in memory.
} processPacket;

// CIRCULAR BUFFER (WRITE FIRST, THEN INCREMENT)
typedef struct {
  processPacket circBuffer[BUFFER_SIZE];
  uint8_t readIndex;
  uint8_t writeIndex;
} packetBuffer;

// GLOBAL STRUCTS
volatile packetBuffer RXbuffer;
volatile packetBuffer TXbuffer;

unsigned long testTimer = 0;



void setup() {
  // put your setup code here, to run once:
  serialCon.begin(115200);
  serialCon.setPacketHandler(&RXpacketPush);

  RXbuffer.readIndex = 0;
  RXbuffer.writeIndex = 0;

  TXbuffer.readIndex = 0;
  TXbuffer.writeIndex = 0;

  InitSynch();

  testTimer = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  serialCon.update();  // CALL THIS FUNCTION ONLY ONCE PER LOOP

  if (TXqueSize() != 0) {
    TXpacketSend();
  }

  if (RXqueSize() != 0){
    
    char testMsg[64];
    processPacket* curRX = RXgetRead();
    if (curRX->packetType == 'S'){
      sprintf(testMsg, "TIME CHECK:%lu", now());
      SERIAL_SendString(testMsg, 'R');
      setTime(now() + strtol((char*) curRX->data, NULL, 10));
    }

    sprintf(testMsg, "RECEIVED %c%c, TIME: %lu", curRX->packetType, curRX->dataType, now());
    SERIAL_SendString(testMsg, 'R');
    RXpacketPop();
  }

#if PINGTEST
  if (millis() - testTimer > 5000) {

    char testMsg[64];
    //sprintf(testMsg, "M:%lu", millis());
    //SERIAL_SendString(testMsg, 'R');
    
    //sprintf(testMsg, "RX_W: %i, RX_R: %i, RX_S: %i", RXbuffer.writeIndex, RXbuffer.readIndex, RXqueSize());
    //SERIAL_SendString(testMsg, 'R');
    uint8_t SERIAL_SendULong(now(), 'T');
    testTimer = millis();
  }
#endif
}







/*/////////////////////////////////////////////////////////////////////////////////////
      OPERATION FUNCTIONS ////////////////////////////////////////////////////////
*//////////////////////////////////////////////////////////////////////////////////////

void QueryHandler(void);
void CommandHandler(void);

uint8_t Confirm(void) {
}

uint8_t InitSynch(void) {
SERIAL_SendString("TS", 'S');

}



/*/////////////////////////////////////////////////////////////////////////////////////
      HELPER FUNCTIONS ////////////////////////////////////////////////////////////////
*//////////////////////////////////////////////////////////////////////////////////////
processPacket* RXgetRead(void){
   return &RXbuffer.circBuffer[RXbuffer.readIndex];
}

processPacket* TXgetRead(void){
   return &TXbuffer.circBuffer[TXbuffer.readIndex];
}


uint8_t SERIAL_SendString(char* txMes, char processType){
  size_t dataSize = strlen(txMes);
  char* sendPtr = (char*) malloc(dataSize);
  memcpy(sendPtr, txMes, dataSize);

  TXbufferPush(dataSize, processType, 'c', sendPtr);
}

uint8_t SERIAL_SendULong(unsigned long txLNum, char processType){
  size_t dataSize = sizeof(txLNum);
  unsigned long* sendPtr = (unsigned long*) malloc(dataSize);
  memcpy(sendPtr, txLNum, dataSize);

  TXbufferPush(dataSize, processType, 'l', sendPtr);
}





// TX num packets enqued
int TXqueSize(void) {
  if (TXbuffer.readIndex == TXbuffer.writeIndex) {
    return 0;
  } else if (TXbuffer.readIndex == ((TXbuffer.writeIndex + 1) % BUFFER_SIZE)) {
    return BUFFER_SIZE;
  } else {
    return (TXbuffer.writeIndex >= TXbuffer.readIndex) ? TXbuffer.writeIndex - TXbuffer.readIndex : TXbuffer.writeIndex + TXbuffer.readIndex - BUFFER_SIZE;
  }
}

// RX num packets enqued.
int RXqueSize(void) {
  if (RXbuffer.readIndex == RXbuffer.writeIndex) {
    return 0;
  } else if (RXbuffer.readIndex == ((RXbuffer.writeIndex + 1) % BUFFER_SIZE)) {
    return BUFFER_SIZE;
  } else {
    return (RXbuffer.writeIndex >= RXbuffer.readIndex) ? RXbuffer.writeIndex - RXbuffer.readIndex : RXbuffer.writeIndex + RXbuffer.readIndex - BUFFER_SIZE;
  }
}

/*/////////////////////////////////////////////////////////////////////////////////////
      SERIAL CONTROL FUNCTIONS ////////////////////////////////////////////////////////
*//////////////////////////////////////////////////////////////////////////////////////

/*
THESE FUNCTIONS SHOULD NEVER BE USED DIRECTLY. This is because they require some more specific setup and conditions
that are better and more consistently handled through helper functions. If possible, stick to using helper functions
*/

// RX ENQUE FUNCTION
// ALLOCATION HERE
void RXpacketPush(const uint8_t *buffer, size_t size) {
  if ((RXbuffer.writeIndex + 1) % BUFFER_SIZE == RXbuffer.readIndex) {
    return;  // ERROR, BUFFER FULL
  }
  // WRITE FIRST, MOVE NEXT
  // DEFINING DATA OF PROCESS PACKET
  RXbuffer.circBuffer[RXbuffer.writeIndex].packetType = buffer[0];  // Query (Q), Command (C), Confirmation (R)
  RXbuffer.circBuffer[RXbuffer.writeIndex].dataType = buffer[1];    // long (l), int (i), char(c)
  RXbuffer.circBuffer[RXbuffer.writeIndex].dataSize = size - 2;

  uint8_t *dataPtr = malloc(size - 2);
  if (dataPtr == NULL) {
    // DE-ALLOCATION ON MALLOC FAILURE
    SERIAL_SendString("MALLOC_FAILURE\r\n", 'R');
    free(dataPtr);
    return;
  }

  void* tempPtr = &buffer[2];
  memcpy(dataPtr, tempPtr, size);

  RXbuffer.circBuffer[RXbuffer.writeIndex].data = dataPtr;
  RXbuffer.writeIndex = ((RXbuffer.writeIndex + 1) % BUFFER_SIZE);
  return;
}

// RX PACKET DEQUE
// DE-ALLOCATION HERE
uint8_t RXpacketPop(void) {
  if (RXbuffer.readIndex == RXbuffer.writeIndex) {
    return 1;  // ERROR, BUFFER EMPTY
  }
  // DEALLOCATION
  free(RXbuffer.circBuffer[RXbuffer.readIndex].data);
  // UPDATING CIRCULAR BUFFER
  RXbuffer.readIndex = ((RXbuffer.readIndex + 1) % BUFFER_SIZE);
  return 0;
}

//TX ENQUE
// ALLOCATE DATA PRIOR TO SENDING TO THIS FUNCTION
uint8_t TXbufferPush(size_t dataLength, uint8_t processType, uint8_t dataType, void *data) {
  if ((TXbuffer.writeIndex + 1) % BUFFER_SIZE == TXbuffer.readIndex) {
    return 1;  // ERROR, BUFFER FULL
  }

  // Assume data is already allocated.

  TXbuffer.circBuffer[TXbuffer.writeIndex].dataSize = dataLength;
  TXbuffer.circBuffer[TXbuffer.writeIndex].packetType = processType;
  TXbuffer.circBuffer[TXbuffer.writeIndex].dataType = dataType;
  TXbuffer.circBuffer[TXbuffer.writeIndex].data = data;

  TXbuffer.writeIndex = ((TXbuffer.writeIndex + 1) % BUFFER_SIZE);
  return 0;
}

//TX SEND (POP)
// DE-ALLOCATES DATA CONTAINED IN PACKETS
uint8_t TXpacketSend(void) {
  if (TXbuffer.readIndex == TXbuffer.writeIndex) {
    return 1;  // ERROR, BUFFER EMPTY
  }
  size_t dataSize = TXbuffer.circBuffer[TXbuffer.readIndex].dataSize;

  char sendData[dataSize + 2];

  // PROCESS TYPE AND DATA TYPE
  sendData[0] = TXbuffer.circBuffer[TXbuffer.readIndex].packetType;
  sendData[1] = TXbuffer.circBuffer[TXbuffer.readIndex].dataType;
  
  // DECLARING SEND DATA
  void* tempPtr = &sendData[2];
  memcpy(tempPtr, TXbuffer.circBuffer[TXbuffer.readIndex].data, dataSize);

  // SERIAL FUNCTION DATA SEND
  serialCon.send(sendData, dataSize);
  // DEALLOCATION
  free(TXbuffer.circBuffer[TXbuffer.readIndex].data);
  TXbuffer.readIndex = ((TXbuffer.readIndex + 1) % BUFFER_SIZE);
  return 0;
}

