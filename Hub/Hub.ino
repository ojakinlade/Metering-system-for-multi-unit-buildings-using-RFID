#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include "keypad.h"
#include "hc12.h"
#include "hmi.h"

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

//RTOS Handle(s)
TaskHandle_t nodeTaskHandle;
QueueHandle_t nodeToAppQueue;

typedef struct
{
  uint16_t node1Pwr;
  uint16_t node1Kwh;
  uint16_t node2Pwr;
  uint16_t node2Kwh;
}pwr_t;

void setup() {
  // put your setup code here, to run once:
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  //Create Tasks
  xTaskCreatePinnedToCore(ApplicationTask,"",30000,NULL,1,NULL,ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(NodeTask,"",7000,NULL,1,&nodeTaskHandle,ARDUINO_RUNNING_CORE);
  nodeToAppQueue = xQueueCreate(1,sizeof(pwr_t));
  if(nodeToAppQueue != NULL)
  {
    Serial.println("--Node-Application Queue created successfully");
  }
  else
  {
    Serial.println("--Node-Application Queue creation failed");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

void ApplicationTask(void* pvParameters)
{ 
  uint8_t rowPins[NUMBER_OF_ROWS] = {2,4,12,13};
  uint8_t columnPins[NUMBER_OF_COLUMNS] = {14,15,25,26};
  static Keypad keypad(rowPins,columnPins);
  static LiquidCrystal_I2C lcd(0x27,20,4);
  static MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
  static HMI hmi(&lcd,&keypad,&rfid); 
  MFRC522::MIFARE_Key key;
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init RC522 

  hmi.RegisterCallback(ValidateRfidTag);
  hmi.RegisterCallback(GetPowerParam);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(2,0);
  lcd.print("Metering System");
  lcd.setCursor(8,1);
  lcd.print("For");
  lcd.setCursor(0,2);
  lcd.print("Multi-unit buildings");
  lcd.setCursor(5,3);
  lcd.print("using RFID");
  delay(2000);
  lcd.clear();
  lcd.print("--Initializing....");
  delay(2000);
  lcd.clear();
//  lcd.clear();
//  lcd.setCursor(0,0);
//  lcd.print("Place RFID tag on");
//  lcd.setCursor(0,1);
//  lcd.print("RFID scanner...");
//  lcd.blink();
  //vTaskResume(nodeTaskHandle);
   
  for(;;)
  {
    hmi.Execute();
    vTaskDelay(pdMS_TO_TICKS(50));

//    
//      if(unitNumber != INVALID)
//      {
//        switch(unitNumber)
//        {
//          case UNIT1:
//            Serial.println("UNIT1");
//          break;
//    
//          case UNIT2:
//            Serial.println("UNIT2");
//          break;
//        }
//      }
//      else
//        Serial.println("INVALID");
//    }
  } 
}

void NodeTask(void* pvParameters)
{
  vTaskSuspend(NULL);
  const uint8_t setPin = 15;
  static HC12 hc12(&Serial2,setPin);
  static pwr_t pwr;
  uint8_t node = HC12::Node1Addr;
  uint32_t prevTime = millis();
  hc12.SetChannel(CHANNEL_15);
  
  for(;;)
  {
    if(millis() - prevTime >= 1500)
    {
      hc12.EncodeData(node,HC12::TxDataId::DEST_ADDR);
      hc12.EncodeData(HC12::QUERY,HC12::TxDataId::DATA_QUERY);
      hc12.TransmitData();
      node = (node == HC12::Node1Addr) ? HC12::Node2Addr : HC12::Node1Addr;
      prevTime = millis();
    }
    if(hc12.ReceivedData())
    {
      if(hc12.DecodeData(HC12::RxDataId::DATA_ACK) == HC12::ACK)
      {
        if(hc12.DecodeData(HC12::RxDataId::SRC_ADDR) == HC12::Node1Addr)
        {
          Serial.println("--Data received from Node 1");
          pwr.node1Pwr = hc12.DecodeData(HC12::RxDataId::POWER);
          pwr.node1Kwh = hc12.DecodeData(HC12::RxDataId::ENERGY);
          //Debug
          Serial.print("Node 1 Pwr: ");
          Serial.println(pwr.node1Pwr);
          Serial.print("Node 1 Kwh: ");
          Serial.println(pwr.node1Kwh);

          if(xQueueSend(nodeToAppQueue,&pwr,0) == pdPASS)
          {
            Serial.println("--Data from Node 1 successfully sent to Application Task");
          }
          else
          {
            Serial.println("--Sending from Node 1 data to Application Task failed");
          }
        }
        else if(hc12.DecodeData(HC12::RxDataId::SRC_ADDR) == HC12::Node2Addr)
        {
          Serial.println("--Data received from Node 2");
          pwr.node2Pwr = hc12.DecodeData(HC12::RxDataId::POWER);
          pwr.node2Kwh = hc12.DecodeData(HC12::RxDataId::ENERGY);
          //Debug
          Serial.print("Node 2 Pwr: ");
          Serial.println(pwr.node2Pwr);
          Serial.print("Node 2 Kwh: ");
          Serial.println(pwr.node2Kwh);

          if(xQueueSend(nodeToAppQueue,&pwr,0) == pdPASS)
          {
            Serial.println("--Data from Node 2 successfully sent to Application Task");
          }
          else
          {
            Serial.println("--Sending from Node 2 data to Application Task failed");
          }
        }
      }
    }
  }
}

/**
 * @brief Selects the Unit whose card is present on the scanner
 * @param
 * @param
 * @return Returns the unit index whose card is present on the scanner
*/
UnitIndex ValidateRfidTag(uint8_t* rfidTagBuffer,uint8_t bufferSize)
{
  UnitIndex unit = UNIT_UNKNOWN;
  for(uint8_t i = 0; i < bufferSize; i++)
  {
    if(rfidTagBuffer[i] == unit1_ID[i])
    {
      unit = UNIT1;
    }
    else if(rfidTagBuffer[i] == unit2_ID[i])
    {
      unit = UNIT2;
    }
    else
    {
      unit = UNIT_UNKNOWN;
    }
  }
  return unit;    
}

/**
 * @brief
 * @param
 * @param
*/
void GetPowerParam(UnitIndex unitIndex,float* pwrPtr,float* kwhPtr)
{
  
}
