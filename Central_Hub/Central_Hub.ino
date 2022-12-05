#include <SPI.h>
#include <MFRC522.h>
#include "hc12.h"
#include "keypad.h"

/*
 * Pinouts
 * RFID      |      Arduino Nano
 * VCC       |        3.3v
 * GND       |        GND
 * NSS       |        D10
 * RST       |        D7
 * MOSI      |        D11
 * MISO      |        D12
 * SCK       |        D13
*/
#define SS_PIN 10
#define RST_PIN 7
const char unit1_ID[10] = "64FA7689";
const char unit2_ID[10] = "2C5AAE49";
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
char rfidBuffer[10] = {0};
int unitNumber = -1; 
MFRC522::MIFARE_Key key; 

enum unitNumber_t
{
  UNIT1 = 0,
  UNIT2
};

/*
 * @brief: Selects the Unit whose card is present on the scanner
 * @return: Returns the unit number whose card is present on the scanner
*/
unitNumber_t selectCardPresent(void)
{
  int unit = -1;
  return unit;
}
 
void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init RC522 
}
 
void loop() {
  if(rfid.PICC_IsNewCardPresent())
  {
    if(rfid.PICC_ReadCardSerial())
    {
      Serial.print(F("RFID Tag UID:"));
     // memcpy(rfidBuffer, rfid.uid.uidByte, rfid.uid.size);
      convertIdToStr(rfid.uid.uidByte, rfid.uid.size);
      unitNumber = selectCardPresent();
      Serial.println("");
      rfid.PICC_HaltA(); // Halt PICC
    }
  }
  if(unitNumber != -1)
  {
    switch(unitNumber)
    {
      case UNIT1:

      break;

      case UNIT2:
      
      break;
    }
  }
  
}

void convertIdToStr(uint8_t* IdBuffer, uint8_t bufferSize) {
  for (uint8_t i = 0; i < bufferSize; i++)
  {
    Serial.print(IdBuffer[i] < 0x10 ? " 0" : " ");
    Serial.print(IdBuffer[i], HEX);
  }
}
