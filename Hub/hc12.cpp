#include <Arduino.h>
#include "hc12.h"

#define RX_DATA_EXPECTED_LEN 8 
/*
 * @brief Initializes HC12 transceiver
*/
HC12::HC12(HardwareSerial* serial,uint8_t Pin,
         uint32_t baudRate,
         int8_t hc12Tx,
         int8_t hc12Rx)
{
  //Initialize private variables
  port = serial;
  setPin = Pin;
  port->begin(baudRate,SERIAL_8N1,hc12Tx,hc12Rx);
//  pinMode(setPin, OUTPUT);
//  digitalWrite(setPin, HIGH);
  rxDataCounter = 0;
  for(uint8_t i = 0; i < BufferSize::TX; i++)
  {
    txBuffer[i] = 0;
  }
  for(uint8_t i = 0; i < BufferSize::RX; i++)
  {
    rxBuffer[i] = 0;
  }
}

/**
 * @brief Encodes data into the "txBuffer" before
 * transmission to an HC12 receiver can occur.
 * @param dataToEncode: data to be encoded
 * @param id: location of specific data within "txBuffer".
*/
void HC12::EncodeData(uint16_t dataToEncode,TxDataId id)
{
  uint8_t dataID = (uint8_t)id;
  txBuffer[dataID] = (dataToEncode & 0xFF00) >> 8;
  txBuffer[dataID + 1] = (dataToEncode & 0x00FF);
}

/**
 * @brief Sets the HC12 transceiver to a particular channel
 * @param AT_Cmd: AT Command to set the HC12 to a particular channel
*/
void HC12::SetChannel(const char* AT_Cmd)
{
  digitalWrite(setPin, LOW);
  vTaskDelay(pdMS_TO_TICKS(40)); //wait for command mode to engage
  port->print(AT_Cmd);
  while(port->available() == 0){} //wait for response
  while(port->available() > 0)
  {
    port->read();
  }
  digitalWrite(setPin,HIGH);
  vTaskDelay(pdMS_TO_TICKS(80));
}

/**
 * @brief Sends data to an HC12 receiver.
*/
void HC12::TransmitData(void)
{
  Serial.print("TX Buffer: ");
  for(uint8_t i = 0; i < BufferSize::TX; i++)
  {
    port->write(txBuffer[i]);
    //Debug
    Serial.print(txBuffer[i]);
    Serial.print(' ');
  }
  Serial.print("\n");
}

/**
 * @brief Receives data from an HC12 transmitter.
 * @reurn true: if total data expected is received completely
 *        false: if otherwise
*/
bool HC12::ReceivedData(void)
{
  bool rxDone = false;
  while(port->available())
  {
    rxBuffer[rxDataCounter] = port->read();
    rxDataCounter++;
  }
  if(rxDataCounter == RX_DATA_EXPECTED_LEN)
  {
    rxDataCounter = 0;
    rxDone = true;
  }

  return rxDone;
}

/**
 * @brief Decodes data received from an HC12 transmitter
 * @param id: location of specific data within "rxBuffer" to be decoded.
 * @return decoded data i.e actual value of desired parameter
*/
uint16_t HC12::DecodeData(RxDataId id)
{
  uint8_t dataID = (uint8_t)id;
  return rxBuffer[dataID] << 8 | rxBuffer[dataID + 1];
}
