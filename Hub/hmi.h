#pragma once

enum UnitIndex
{
  UNIT1 = 0,
  UNIT2,
  UNIT_UNKNOWN
};

class HMI
{
  private:
    enum State{ST_MENUPAGE,ST_PLACETAGPAGE,ST_PWRINFOPAGE,ST_USERGUIDEPAGE};
    //Objects
    LiquidCrystal_I2C* lcdPtr;
    Keypad* keypadPtr;
    //Function pointers
    UnitIndex(*ValidateRfidTag)(uint8_t*,uint8_t);
    void(*GetPowerParam)(UnitIndex,float*,float*);
    State currentState;
    //Display Methods
    void DisplayMenuPage(void);
    void DisplayRowHeadings(char** heading);
    void DisplayUserGuidePage(void);
    void DisplayPlaceTagPage(void);
    //State Methods
    void ChangeStateTo(State nextState);
    void StateFunc_MenuPage(void);
    void StateFunc_PwrInfoPage(void);
    void StateFunc_PlaceTagPage(void);
    void StateFunc_UserGuidePage(void);
  public:
    HMI(LiquidCrystal_I2C* lcdPtr,Keypad* keypadPtr);
    void Execute(void);
    void RegisterCallback(UnitIndex(*ValidateRfidTag)(uint8_t*,uint8_t));
    void RegisterCallback(void(*GetPowerParam)(UnitIndex,float*,float*));
};
