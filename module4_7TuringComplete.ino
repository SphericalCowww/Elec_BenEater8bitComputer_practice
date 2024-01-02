#define SHIFTER_DATA  2
#define SHIFTER_CLK   3
#define SHIFTER_LATCH 4

#define EEPROM_D0            5
#define EEPROM_D7           12
#define EEPROM_WRITE_ENABLE 13

#define HLT 0b1000000000000000 //(halt)
#define MI  0b0100000000000000 //(RAM memeory address in)
#define RI  0b0010000000000000 //(RAM memeory content in)
#define RO  0b0001000000000000 //(RAM memeory content out)
#define IO  0b0000100000000000 //(instruction out)
#define II  0b0000010000000000 //(instruction in)
#define AI  0b0000001000000000 //(A register in)
#define AO  0b0000000100000000 //(A register out)
#define EO  0b0000000010000000 //(ALU sum in)
#define SU  0b0000000001000000 //(ALU subtract)
#define BI  0b0000000000100000 //(B register in)
#define OI  0b0000000000010000 //(output display)
#define CE  0b0000000000001000 //(counter enable)
#define CO  0b0000000000000100 //(counter out)
#define J   0b0000000000000010 //(counter in, or jump)
#define FI  0b0000000000000001 //(carry flag(CF)/zero flag(ZF) in)
////instruction map
const PROGMEM uint16_t INSTRUCTION_TEMPLATE[16][8] = {
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0}, //0000 - NOP (no operation)
  {MI|CO, RO|II|CE, IO|MI, RO|AI, 0,           0, 0, 0}, //0001 - LDA (load register A from RAM)
  {MI|CO, RO|II|CE, IO|MI, RO|BI, EO|AI|FI,    0, 0, 0}, //0010 - ADD (adding A and B)
  {MI|CO, RO|II|CE, IO|MI, RO|BI, EO|AI|SU|FI, 0, 0, 0}, //0011 - SUB (subtracting A from B)
  {MI|CO, RO|II|CE, IO|MI, AO|RI, 0,           0, 0, 0}, //0100 - STA (store registor A)
  {MI|CO, RO|II|CE, IO|AI, 0,     0,           0, 0, 0}, //0101 - LDI (load register A from instruction)
  {MI|CO, RO|II|CE, IO|J,  0,     0,           0, 0, 0}, //0110 - JMP (jump to instruction)
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0}, //0111 - JC  (jump on carry)
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0}, //1000 - JC  (jump on zero)
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0}, //1001
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0}, //1010
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0}, //1011
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0}, //1100
  {MI|CO, RO|II|CE, 0,     0,     0,           0, 0, 0}, //1101
  {MI|CO, RO|II|CE, AO|OI, 0,     0,           0, 0, 0}, //1110 - OUT (display output)
  {MI|CO, RO|II|CE, HLT,   0,     0,           0, 0, 0}, //1111 - HLT (halt)
};
uint16_t INSTRUCTION_MAP[4][16][8];
void initInstructionMap() {
  memcpy_P(INSTRUCTION_MAP[0], INSTRUCTION_TEMPLATE, sizeof(INSTRUCTION_TEMPLATE)); //ZF, CF = 0, 0
  memcpy_P(INSTRUCTION_MAP[1], INSTRUCTION_TEMPLATE, sizeof(INSTRUCTION_TEMPLATE)); //ZF, CF = 0, 1
  memcpy_P(INSTRUCTION_MAP[2], INSTRUCTION_TEMPLATE, sizeof(INSTRUCTION_TEMPLATE)); //ZF, CF = 1, 0
  memcpy_P(INSTRUCTION_MAP[3], INSTRUCTION_TEMPLATE, sizeof(INSTRUCTION_TEMPLATE)); //ZF, CF = 1, 1
  INSTRUCTION_MAP[1][0b0111][2] = IO|J;
  INSTRUCTION_MAP[2][0b1000][2] = IO|J;
  INSTRUCTION_MAP[3][0b0111][2] = IO|J;
  INSTRUCTION_MAP[3][0b1000][2] = IO|J;
};
  
void printEEPROM(int rangeLower, int rangeUpper);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  initInstructionMap();
  delay(200);
  pinMode(SHIFTER_DATA,  OUTPUT);
  pinMode(SHIFTER_CLK,   OUTPUT);
  pinMode(SHIFTER_LATCH, OUTPUT);
  digitalWrite(EEPROM_WRITE_ENABLE, HIGH);      //HIGH to disable
  pinMode(EEPROM_WRITE_ENABLE, OUTPUT);         //Notice the order considering the pullup resistor
  Serial.begin(9600);

  Serial.println("Clearing EEPROM:");
  clearEEPROM();  
  printEEPROM(0, 3*256);

  int mapLength = 1024;
  int flagVal = 0;
  int instructionVal = 0;
  int stepVal = 0;
  int byteSelect = 0;
  Serial.println("Mapping EEPROM unsigned..."); 
  for(int addrVal = 0; addrVal < mapLength; addrVal += 1) {
    flagVal        = (addrVal & 0b1100000000) >> 8;
    instructionVal = (addrVal & 0b0001111000) >> 3;
    stepVal        = (addrVal & 0b0000000111);
    byteSelect     = (addrVal & 0b0010000000) >> 7;
    if (byteSelect == 1) {
      writeEEPROM(addrVal, INSTRUCTION_MAP[flagVal][instructionVal][stepVal]);
    } else {
      writeEEPROM(addrVal, INSTRUCTION_MAP[flagVal][instructionVal][stepVal] >> 8);
    }
    if (addrVal % 64 == 0) {
      Serial.println(addrVal);
    }
  }

  printEEPROM(0, 1024);

  digitalWrite(EEPROM_WRITE_ENABLE, HIGH);
  setAddress(1, true);
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
    if (address % 64 == 0){
      Serial.println(address);
    }
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
