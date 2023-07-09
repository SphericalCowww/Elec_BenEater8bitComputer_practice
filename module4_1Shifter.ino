#define SHIFTER_DATA  2
#define SHIFTER_CLK   3
#define SHIFTER_LATCH 4

#define EEPROM_D0 5
#define EEPROM_D7 12


void setup() {
    pinMode(SHIFTER_DATA,  OUTPUT);
    pinMode(SHIFTER_CLK,   OUTPUT);
    pinMode(SHIFTER_LATCH, OUTPUT);

    setAddress(0xffff, true);
}
void loop() {
}
//////////////////////////////////////////////////////////
void setAddress(int address, bool outputEnable) {
    if(outputEnable == true) {
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
