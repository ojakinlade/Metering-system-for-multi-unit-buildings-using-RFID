#include <SoftwareSerial.h>
#include <PZEM004Tv30.h>
#include "hc12.h"

namespace Pin
{
  const uint8_t setPin = 5;
  const uint8_t hc12Tx = 2;
  const uint8_t hc12Rx = 3 ;
  const uint8_t pzemTx = 8;
  const uint8_t pzemRx = 9;
};

SoftwareSerial hc12Serial(Pin::hc12Tx,Pin::hc12Rx);
SoftwareSerial pzemSerial(Pin::pzemTx,Pin::pzemRx);
PZEM004Tv30 pzem(pzemSerial);
HC12 hc12(&hc12Serial,Pin::setPin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Smart Metering!!!");
  hc12Serial.listen();
  hc12.SetChannel(CHANNEL_15);
}

void loop() {
  // put your main code here, to run repeatedly:  
  if(hc12.ReceivedData())
  {
    if(hc12.DecodeData(HC12::RxDataId::DATA_QUERY) == HC12::QUERY)
    {
      if(hc12.DecodeData(HC12::RxDataId::RX_ADDR) == HC12::deviceAddr)
      {
        Serial.println("--Node 1: Query Received");
        pzemSerial.listen();
//        uint16_t pwr = pzem.power() * 10;
//        uint16_t kwh = pzem.energy() * 1000;

        uint16_t pwr = 50 * 10;
        uint16_t kwh = 0.6 * 1000;
        //Debug
        Serial.print("PWR: ");
        Serial.println(pwr);
        Serial.print("KWH: ");
        Serial.println(kwh);
        hc12.EncodeData(HC12::ACK,HC12::TxDataId::DATA_ACK);
        hc12.EncodeData(HC12::deviceAddr,HC12::TxDataId::SRC_ADDR);
        hc12.EncodeData(pwr,HC12::TxDataId::POWER);
        hc12.EncodeData(kwh,HC12::TxDataId::ENERGY);

        hc12.TransmitData();
        hc12Serial.listen();
      }
    }
  }

}
