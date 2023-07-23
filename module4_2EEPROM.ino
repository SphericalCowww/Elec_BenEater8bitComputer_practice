#define SHIFTER_DATA  2
#define SHIFTER_CLK   3
#define SHIFTER_LATCH 4

#define EEPROM_D0            5
#define EEPROM_D7           12
#define EEPROM_WRITE_ENABLE 13

byte displayMap[] = {0x01, 0x4f, 0x12, 0x06, 0x4c, 0x24, 0x20, 0x0f,
                     0x00, 0x04, 0x08, 0x60, 0x31, 0x42, 0x30, 0x38};
void printEEPROM(int rangeLower, int rangeUpper);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  delay(200);
  pinMode(SHIFTER_DATA,  OUTPUT);
  pinMode(SHIFTER_CLK,   OUTPUT);
  pinMode(SHIFTER_LATCH, OUTPUT);

  digitalWrite(EEPROM_WRITE_ENABLE, HIGH);
  pinMode(EEPROM_WRITE_ENABLE, OUTPUT);

  Serial.begin(9600);

  clearEEPROM();
//  writeEEPROM(1, 0xff);
//  printEEPROM(0, 31);
  for(int address = 0; address <= 15; address += 1) {
    writeEEPROM(address, displayMap[address]);
  }
  printEEPROM(0, 31);
}
void loop() {}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setAddress(int address, bool EEPROM_OUTPUT_ENABLE) {
  if(EEPROM_OUTPUT_ENABLE == true) {
    bitWrite(address, 15, LOW);
  }
  else {
    bitWrite(address, 15, HIGH);
  }
  shiftOut(SHIFTER_DATA, SHIFTER_CLK, MSBFIRST, (address >> 8));
  shiftOut(SHIFTER_DATA, SHIFTER_CLK, MSBFIRST, address);
  digitalWrite(SHIFTER_LATCH, LOW);
  digitalWrite(SHIFTER_LATCH, HIGH);
  digitalWrite(SHIFTER_LATCH, LOW);
}
byte readEEPROM(int address) {
  for (int pin = EEPROM_D7; EEPROM_D0 <= pin; pin -= 1) {
    pinMode(pin, INPUT);
  }
  
  setAddress(address, true);
  int dataEEPROM = 0;
  for (int pin = EEPROM_D7; EEPROM_D0 <= pin; pin -= 1) {
    dataEEPROM = (dataEEPROM << 1) + digitalRead(pin);
  }
  return dataEEPROM;
}
void writeEEPROM(int address, byte data) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, OUTPUT);
  }
  
  setAddress(address, false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, data & 1);  
    data = data >> 1;
  }
  digitalWrite(EEPROM_WRITE_ENABLE, LOW);
  delayMicroseconds(1);
  digitalWrite(EEPROM_WRITE_ENABLE, HIGH);
  delay(5);
}
void clearEEPROM() {
  for(int address = 0; address <= 2047; address += 1) {
    writeEEPROM(address, 0xff);
  }
}
void printEEPROM(int rangeLower=0, int rangeUpper=2047) {
  for(int base = rangeLower; base <= rangeUpper; base += 16) {
    byte data[16];
    for(int shift = 0; shift <= 15; shift += 1) {
      data[shift] = readEEPROM(base + shift);
    }

    char output[80];
    sprintf(output, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1],  data[2],  data[3],  data[4],  data[5],  data[6],  data[7],  data[8],
                  data[9], data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17]);
    Serial.println(output);
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
