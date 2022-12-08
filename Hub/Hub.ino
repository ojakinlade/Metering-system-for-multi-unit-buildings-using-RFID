#include <SPI.h>
#include <MFRC522.h>

//RTOS
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE  0
#else
#define ARDUINO_RUNNING_CORE  1
#endif

/*
 * Pinouts
 * RFID      |      ESP32
 * VCC       |        3.3v
 * GND       |        GND
 * NSS       |        5
 * RST       |        27
 * MOSI      |        23
 * MISO      |        19
 * SCK       |        18
*/

#define SS_PIN 5
#define RST_PIN 27

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

void Get_TagID(uint8_t* IdBuffer, uint8_t bufferSize) {
  for (uint8_t i = 0; i < bufferSize; i++)
  {
    rfidBuffer[i] = IdBuffer[i];
//    Serial.print(IdBuffer[i] < 0x10 ? " 0" : " ");
//    Serial.print(IdBuffer[i], HEX);
  }
}

void setup() {
  // put your setup code here, to run once:
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init RC522 
  //Create Tasks
  xTaskCreatePinnedToCore(ApplicationTask,"",7000,NULL,1,NULL,ARDUINO_RUNNING_CORE);
  // xTaskCreatePinnedToCore(SensorsTask,"",7000,NULL,1,NULL,ARDUINO_RUNNING_CORE);
  // xTaskCreatePinnedToCore(MqttTask,"",7000,NULL,1,NULL,ARDUINO_RUNNING_CORE);

}

void loop() {
  // put your main code here, to run repeatedly:

}

void ApplicationTask(void* pvParameters)
{

  for(;;)
  {
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
}




