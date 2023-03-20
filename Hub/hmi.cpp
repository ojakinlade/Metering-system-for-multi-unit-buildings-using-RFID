#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> //Version 1.1.2
#include "keypad.h"
#include "hmi.h"

const uint8_t numOfRows = 4;

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
      HMI::ChangeStateTo(ST_PWRINFOPAGE);
      break;
    case '2':
      HMI::ChangeStateTo(ST_USERGUIDEPAGE);
      break;
  }
}

HMI::HMI(LiquidCrystal_I2C* lcdPtr,Keypad* keypadPtr)
{
  //Initialize private variables
  this->lcdPtr = lcdPtr;
  this->keypadPtr = keypadPtr;
  currentState = ST_MENUPAGE;
}


void HMI::Execute(void)
{
  switch(currentState)
  {
    case ST_MENUPAGE:
      HMI::StateFunc_MenuPage();
      break;
    case ST_USERGUIDEPAGE:
      HMI::StateFunc_UserGuidePage();
      break;
  }
}
