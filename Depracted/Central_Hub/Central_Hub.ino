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
const uint8_t unit1_ID[4] = {0x64,0xFA,0x76,0x89};
const uint8_t unit2_ID[4] = {0x2C,0x5A,0xAE,0x49};
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
uint8_t rfidBuffer[4] = {0};
int unitNumber = -1; 
MFRC522::MIFARE_Key key; 

enum unitNumber_t
{
  INVALID = -1,
  UNIT1 = 0,
  UNIT2
};

/*
 * @brief: Selects the Unit whose card is present on the scanner
 * @return: Returns the unit number whose card is present on the scanner
*/
unitNumber_t selectCardPresent(void)
{
  unitNumber_t unit = INVALID;
  for(uint8_t i = 0; i < 4; i++)
  {
    if(rfidBuffer[i] == unit1_ID[i])
    {
      unit = UNIT1;
    }
    else if(rfidBuffer[i] = unit2_ID[i])
    {
      unit = UNIT2;
    }
    else
    {
      unit = INVALID;
    }
  }
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
      Get_TagID(rfid.uid.uidByte, rfid.uid.size);
      unitNumber = selectCardPresent();
      Serial.println("");
      rfid.PICC_HaltA(); // Halt PICC
    }
  if(unitNumber != INVALID)
  {
    switch(unitNumber)
    {
      case UNIT1:
        Serial.println("UNIT1");
      break;

      case UNIT2:
        Serial.println("UNIT2");
      break;
    }
  }
  }

  
}

void Get_TagID(uint8_t* IdBuffer, uint8_t bufferSize) {
  for (uint8_t i = 0; i < bufferSize; i++)
  {
    rfidBuffer[i] = IdBuffer[i];
  }
}
