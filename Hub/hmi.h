#pragma once

#define RFID_BUFFER_SIZE  4

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
    
    //Objects and variables
    LiquidCrystal_I2C* lcdPtr;
    MFRC522* rfidPtr;
    Keypad* keypadPtr;
    uint8_t rfidBuffer[RFID_BUFFER_SIZE];
    float pwr;
    float kwh;
    UnitIndex unitIndex;
    State currentState;
    
    //Function pointers
    UnitIndex(*ValidateRfidTag)(uint8_t*,uint8_t);
    void(*GetPowerParam)(UnitIndex,float*,float*);
    
    //RFID methods
    void Get_TagID(uint8_t* IdBuffer, uint8_t bufferSize); 
    //Display Methods
    void DisplayMenuPage(void);
    void DisplayRowHeadings(char** heading);
    void DisplayUserGuidePage(void);
    void DisplayPlaceTagPage(void);
    void DisplayLoginSuccess(void);
    void DisplayLoginError(void);
    void DisplayPwrInfoPage(void);
    //State Methods
    void ChangeStateTo(State nextState);
    void StateFunc_MenuPage(void);
    void StateFunc_PwrInfoPage(void);
    void StateFunc_PlaceTagPage(void);
    void StateFunc_UserGuidePage(void);
  public:
    HMI(LiquidCrystal_I2C* lcdPtr,Keypad* keypadPtr,MFRC522* rfidPtr);
    void Execute(void);
    void RegisterCallback(UnitIndex(*ValidateRfidTag)(uint8_t*,uint8_t));
    void RegisterCallback(void(*GetPowerParam)(UnitIndex,float*,float*));
};
