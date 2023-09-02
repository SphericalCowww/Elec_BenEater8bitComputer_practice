#define SHIFTER_DATA  2
#define SHIFTER_CLK   3
#define SHIFTER_LATCH 4

#define EEPROM_D0            5
#define EEPROM_D7           12
#define EEPROM_WRITE_ENABLE 13

////common anode 7-segment LED display
//byte displayMap[] = {0x01, 0x4f, 0x12, 0x06, 0x4c, 0x24, 0x20, 0x0f,
//                     0x00, 0x04, 0x08, 0x60, 0x31, 0x42, 0x30, 0x38};
//common cathode 7-segment LED display
byte displayMap[] = {0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70,
                     0x7f, 0x7b, 0x77, 0x1f, 0x4e, 0x3d, 0x4f, 0x47};
void printEEPROM(int rangeLower, int rangeUpper);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  delay(200);
  pinMode(SHIFTER_DATA,  OUTPUT);
  pinMode(SHIFTER_CLK,   OUTPUT);
  pinMode(SHIFTER_LATCH, OUTPUT);
  digitalWrite(EEPROM_WRITE_ENABLE, HIGH);      //HIGH to disable
  pinMode(EEPROM_WRITE_ENABLE, OUTPUT);         //Notice the order considering the pullup resistor
  Serial.begin(9600);

//  Serial.println("Clearing EEPROM:");
//  clearEEPROM();  
//  printEEPROM(0, 3*256);

  int digitN   = 10;
  int addrJump    =  256;      
  int addrJumpNeg = -128;
  int addrJumpPos =  128;
  Serial.println("Mapping EEPROM unsigned..."); //NOTE: the input value only works up to 2^8
  for(int addrVal = 0; addrVal < addrJump; addrVal += 1) {
    writeEEPROM(addrVal + 0*addrJump, displayMap[ addrVal                 %digitN]);
    writeEEPROM(addrVal + 1*addrJump, displayMap[(addrVal/digitN)         %digitN]);
    writeEEPROM(addrVal + 2*addrJump, displayMap[(addrVal/(digitN*digitN))%digitN]);
    writeEEPROM(addrVal + 3*addrJump, 0x00);
  }
  Serial.println("Mapping EEPROM signed...");  //NOTE: (byte) uses two's complement
    for(int addrVal = addrJumpNeg; addrVal < addrJumpPos; addrVal += 1) {
    writeEEPROM((byte)addrVal + 4*addrJump, displayMap[ abs(addrVal)                 %digitN]);
    writeEEPROM((byte)addrVal + 5*addrJump, displayMap[(abs(addrVal)/digitN)         %digitN]);
    writeEEPROM((byte)addrVal + 6*addrJump, displayMap[(abs(addrVal)/(digitN*digitN))%digitN]);
    if (addrVal < 0) {
      writeEEPROM((byte)addrVal + 7*addrJump, 0x01);
    }
    else {
      writeEEPROM((byte)addrVal + 7*addrJump, 0x00);
    }
  }
  printEEPROM(0, 8*addrJump-1);                     //somehow needed for the output

  digitalWrite(EEPROM_WRITE_ENABLE, HIGH);
  setAddress(248, true);
  Serial.println("End of code.");
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
void writeEEPROM(int address, byte data_) {
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    pinMode(pin, OUTPUT);
  }
  setAddress(address, false);
  for (int pin = EEPROM_D0; pin <= EEPROM_D7; pin += 1) {
    digitalWrite(pin, data_ & 1);  
    data_ = data_ >> 1;
  }
  digitalWrite(EEPROM_WRITE_ENABLE, LOW);
  delayMicroseconds(1);
  digitalWrite(EEPROM_WRITE_ENABLE, HIGH);
  delayMicroseconds(5000);
}
void clearEEPROM() {
  for(int address = 0; address < 2048; address += 1) {
    writeEEPROM(address, 0xff);
  }
}
void printEEPROM(int rangeLower=0, int rangeUpper=2047) {
  for(int base = rangeLower; base < rangeUpper; base += 16) {
    byte dataEEPROM[16];
    for(int shift = 0; shift < 16; shift += 1) {
      dataEEPROM[shift] = readEEPROM(base + shift);
      delayMicroseconds(5000);
    }

    char output[80];
    sprintf(output, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x",
            base, dataEEPROM[0],  dataEEPROM[1],  dataEEPROM[2],  dataEEPROM[3],  
                  dataEEPROM[4],  dataEEPROM[5],  dataEEPROM[6],  dataEEPROM[7],  
                  dataEEPROM[8],  dataEEPROM[9],  dataEEPROM[10], dataEEPROM[11], 
                  dataEEPROM[12], dataEEPROM[13], dataEEPROM[14], dataEEPROM[15]);
    Serial.println(output);
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
