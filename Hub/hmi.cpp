#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //Version 1.1.2
#include "keypad.h"
#include "hmi.h"

const uint8_t numOfRows = 4;

void HMI::Get_TagID(uint8_t* IdBuffer, uint8_t bufferSize) 
{
  for (uint8_t i = 0; i < bufferSize; i++)
  {
    rfidBuffer[i] = IdBuffer[i];
    Serial.print(IdBuffer[i] < 0x10 ? " 0" : " ");
    Serial.print(IdBuffer[i], HEX);
  }
  Serial.println();
}

void HMI::ChangeStateTo(State nextState)
{
  currentState = nextState;
  lcdPtr->clear();
}

void HMI::DisplayRowHeadings(char** heading)
{
  for(uint8_t i = 0; i < numOfRows; i++)
  {
    lcdPtr->setCursor(0,i);
    lcdPtr->print(heading[i]);
  }
}

void HMI::DisplayMenuPage(void)
{
  char heading1[] = "        MENU     ";
  char heading2[] = "1. Power Usage Info.";
  char heading3[] = "2. User Guide ";
  char heading4[] = "";
  char* heading[] = {heading1,heading2,heading3,heading4};
  HMI::DisplayRowHeadings(heading);
}

void HMI::DisplayUserGuidePage(void)
{
  char heading1[] = "- Press the number ";
  char heading2[] = "corresponding to the";
  char heading3[] = "desired operation";
  char heading4[] = "- Follow the prompt";
  char* heading[] = {heading1,heading2,heading3,heading4};
  HMI::DisplayRowHeadings(heading);
}

void HMI::DisplayLoginError(void)
{
  lcdPtr->clear();
  lcdPtr->print("LOGIN ERROR!!");
  lcdPtr->setCursor(0,1);
  lcdPtr->print("CARD NOT RECOGNIZED");
}

void HMI::DisplayLoginSuccess(void)
{
  lcdPtr->clear();
  lcdPtr->print("LOGIN SUCCESSFUL!");
  lcdPtr->setCursor(0,1);
  lcdPtr->print("WELCOME UNIT: ");
  lcdPtr->print(unitIndex + 1);
}

void HMI::DisplayPlaceTagPage(void)
{
  char heading1[] = "Place your RFID card";
  char heading2[] = "close to the sensor";
  char heading3[] = "";
  char heading4[] = "";
  char* heading[] = {heading1,heading2,heading3,heading4};
  HMI::DisplayRowHeadings(heading);
}

void HMI::DisplayPwrInfoPage(void)
{
  char heading1[] = "  POWER INFO  ";
  char heading2[] = "PWR: ";
  char heading3[] = "KWH:";
  char heading4[] = "Amount due(N): ";
  char* heading[] = {heading1,heading2,heading3,heading4};
  HMI::DisplayRowHeadings(heading);
}

void HMI::StateFunc_UserGuidePage(void)
{
  HMI::DisplayUserGuidePage();
  char key = keypadPtr->GetChar();
  switch(key)
  {
    case 'B':
      HMI::ChangeStateTo(ST_MENUPAGE);
      break;
  }
}

void HMI::StateFunc_MenuPage(void)
{
  HMI::DisplayMenuPage();
  char key = keypadPtr->GetChar();
  switch(key)
  {
    case '1':
      HMI::ChangeStateTo(ST_PLACETAGPAGE);
      break;
    case '2':
      HMI::ChangeStateTo(ST_USERGUIDEPAGE);
      break;
  }
}

void HMI::StateFunc_PlaceTagPage(void)
{
  HMI::DisplayPlaceTagPage();
  if(rfidPtr->PICC_IsNewCardPresent())
  {
    if(rfidPtr->PICC_ReadCardSerial())
    {
      Serial.print(F("RFID Tag UID:"));
      Get_TagID(rfidPtr->uid.uidByte, rfidPtr->uid.size);
      unitIndex = ValidateRfidTag(rfidBuffer,RFID_BUFFER_SIZE);
      if(unitIndex == UNIT_UNKNOWN)
      {
        HMI::DisplayLoginError();
        vTaskDelay(pdMS_TO_TICKS(3000));
        HMI::ChangeStateTo(ST_MENUPAGE);
      }
      else
      {
        HMI::DisplayLoginSuccess();
        vTaskDelay(pdMS_TO_TICKS(3000));
        HMI::ChangeStateTo(ST_PWRINFOPAGE);
      }
      rfidPtr->PICC_HaltA(); // Halt PICC
    }
  }
  char key = keypadPtr->GetChar();
  switch(key)
  {
    case 'B':
      HMI::ChangeStateTo(ST_MENUPAGE);
      break;
  }
}

void HMI::StateFunc_PwrInfoPage(void)
{
  HMI::DisplayPwrInfoPage();
  GetPowerParam(unitIndex,&pwr,&kwh);
  lcdPtr->setCursor(5,1);
  lcdPtr->print(pwr);
  lcdPtr->print("W");
  lcdPtr->setCursor(5,2);
  lcdPtr->print(kwh);
  lcdPtr->print("KWh");
  char key = keypadPtr->GetChar();
  switch(key)
  {
    case 'B':
      HMI::ChangeStateTo(ST_MENUPAGE);
      break;
  }
}

HMI::HMI(LiquidCrystal_I2C* lcdPtr,Keypad* keypadPtr,MFRC522* rfidPtr)
{
  //Initialize private variables
  this->lcdPtr = lcdPtr;
  this->keypadPtr = keypadPtr;
  this->rfidPtr = rfidPtr;
  currentState = ST_MENUPAGE;
  pwr = 0;
  kwh = 0;
  for(uint8_t i = 0; i < RFID_BUFFER_SIZE; i++)
  {
    rfidBuffer[i] = 0;
  }
}


void HMI::Execute(void)
{
  switch(currentState)
  {
    case ST_MENUPAGE:
      HMI::StateFunc_MenuPage();
      break;
    case ST_PLACETAGPAGE:
      HMI::StateFunc_PlaceTagPage();
      break;
    case ST_USERGUIDEPAGE:
      HMI::StateFunc_UserGuidePage();
      break;
    case ST_PWRINFOPAGE:
      HMI::StateFunc_PwrInfoPage();
      break;
  }
}

void HMI::RegisterCallback(UnitIndex(*ValidateRfidTag)(uint8_t*,uint8_t))
{
  this->ValidateRfidTag = ValidateRfidTag;
  Serial.println("Successfully registered <ValidateRfid> callback");
}

void HMI::RegisterCallback(void(*GetPowerParam)(UnitIndex,float*,float*))
{
  this->GetPowerParam = GetPowerParam;
  Serial.println("Successfully registered <GetPowerParam> callback");
}
