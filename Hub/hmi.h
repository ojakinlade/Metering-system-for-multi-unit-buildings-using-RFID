#pragma once

#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

class HMI
{
  private:
    enum State{ST_MENUPAGE,ST_PWRINFOPAGE,ST_USERGUIDEPAGE};
    //Objects
    LiquidCrystal_I2C* lcdPtr;
    Keypad* keypadPtr;
    State currentState;
    //Display Methods
    void DisplayMenuPage(void);
    void DisplayRowHeadings(char** heading);
    void DisplayUserGuidePage(void);
    //State Methods
    void ChangeStateTo(State nextState);
    void StateFunc_MenuPage(void);
    void StateFunc_UserGuidePage(void);
  public:
    HMI(LiquidCrystal_I2C* lcdPtr,Keypad* keypadPtr);
    void Execute(void);
};
