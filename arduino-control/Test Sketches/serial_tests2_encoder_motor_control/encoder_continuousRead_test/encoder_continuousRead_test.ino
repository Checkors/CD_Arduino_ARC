volatile int count;
ISR(TIMER2_COMPA_vect)
{
count++;
OCR2A = 0x9C;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
    ;

  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 0xF424;
  TCCR1B = (1 << WGM12) | (1 << CS12);
  TIMSK1 = (1 << OCIE1A);

  Serial.print("SERIAL START\r\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    char receiveCon[64];
    sprintf(receiveCon, "RECEIVED: %c\r\n", inByte);
    Serial.print(receiveCon);
  }
}
