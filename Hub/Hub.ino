#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include "keypad.h"
#include "hc12.h"
#include "hmi.h"

//RTOS
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE  0
#else
#define ARDUINO_RUNNING_CORE  1
#endif

#define SS_PIN 5
#define RST_PIN 27

const uint8_t unit1_ID[4] = {0x64,0xFA,0x76,0x89};
const uint8_t unit2_ID[4] = {0x2C,0x5A,0xAE,0x49};

//RTOS Handle(s)
TaskHandle_t nodeTaskHandle;
QueueHandle_t nodeToGetPwrParam;

Preferences preferences;

typedef struct
{
  uint16_t node1Pwr;
  uint16_t node1Kwh;
  uint16_t node2Pwr;
  uint16_t node2Kwh;
  uint16_t node1Units;
  uint16_t node2Units;
}pwr_t;

void setup() {
  // put your setup code here, to run once:
  setCpuFrequencyMhz(80);
  Serial.begin(115200);
  preferences.begin("Metering",false);
//  preferences.putUShort("0",20);
//  preferences.putUShort("2",19);
  //Create Tasks
  xTaskCreatePinnedToCore(ApplicationTask,"",30000,NULL,1,NULL,ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(NodeTask,"",7000,NULL,1,&nodeTaskHandle,ARDUINO_RUNNING_CORE);
  nodeToGetPwrParam = xQueueCreate(1,sizeof(pwr_t));
  if(nodeToGetPwrParam != NULL)
  {
    Serial.println("--Node-GetPowerParam Queue created successfully");
  }
  else
  {
    Serial.println("--Node-GetPowerParam Queue creation failed");
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

  //Callbacks
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
  vTaskResume(nodeTaskHandle);
   
  for(;;)
  {
    hmi.Execute();
    vTaskDelay(pdMS_TO_TICKS(50));
  } 
}

void NodeTask(void* pvParameters)
{
  vTaskSuspend(NULL);
  const uint8_t setPin = 34;
  static HC12 hc12(&Serial2,setPin);
  static pwr_t pwr;
  uint8_t node = HC12::Node1Addr;
  uint32_t prevTime = millis();
 // hc12.SetChannel(CHANNEL_15);
  
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

          if(xQueueSend(nodeToGetPwrParam,&pwr,0) == pdPASS)
          {
            Serial.println("--Data from Node 1 successfully sent to GetPowerParam");
          }
          else
          {
            Serial.println("--Sending from Node 1 data to GetPowerParam failed");
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

          if(xQueueSend(nodeToGetPwrParam,&pwr,0) == pdPASS)
          {
            Serial.println("--Data from Node 2 successfully sent to GetPowerParam");
          }
          else
          {
            Serial.println("--Sending from Node 2 data to GetPowerParam failed");
          }
        }
      }
    }
  }
}

/**
 * @brief Selects the Unit whose card is present on the scanner
 * @param rfidTagBuffer: contains the rfid tag uid
 * @param bufferSize: Size of the rfidTagBuffer
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
void GetPowerParam(UnitIndex unitIndex,float* pwrPtr,float* kwhPtr,float* prevKwhPtr,uint16_t* unitsAvailablePtr)
{
  if(unitIndex == UNIT_UNKNOWN)
  {
    return;
  }
  static pwr_t pwr;
  if(xQueueReceive(nodeToGetPwrParam,&pwr,0) == pdPASS)
  {
    Serial.println("Node-GetPowerParam data received");
  }
  if(unitIndex == UNIT1)
  {
    *pwrPtr = pwr.node1Pwr / 10.0;
    *kwhPtr = pwr.node1Kwh / 1000.0;
    *unitsAvailablePtr = preferences.getUShort("0",0);
    *prevKwhPtr = preferences.getFloat("1",0);
    if((lround(*kwhPtr*1000) - lround(*prevKwhPtr*1000)) >= 1000)
    {
      
      *unitsAvailablePtr = *unitsAvailablePtr - 1;
      *prevKwhPtr = *kwhPtr;
      preferences.putUShort("0",*unitsAvailablePtr);
      preferences.putFloat("1",*prevKwhPtr);
    }
  }
  else if(unitIndex == UNIT2)
  {
    *pwrPtr = pwr.node2Pwr / 10.0;
    *kwhPtr = pwr.node2Kwh / 1000.0;
    *unitsAvailablePtr = preferences.getUShort("2",0);
    *prevKwhPtr = preferences.getFloat("3",0);
    if((lround(*kwhPtr*1000) - lround(*prevKwhPtr*1000)) >= 1000)
    {
      *unitsAvailablePtr = *unitsAvailablePtr - 1;
      *prevKwhPtr = *kwhPtr; 
      preferences.putUShort("2",*unitsAvailablePtr);
      preferences.putFloat("3",*prevKwhPtr);
    }
  } 
}
