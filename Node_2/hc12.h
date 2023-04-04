#pragma once

#define CHANNEL_15 "AT+C015\r\n"

class HC12
{
  private:
    enum BufferSize { TX = 8, RX = 8};
    SoftwareSerial* port;
    uint8_t setPin;
    uint8_t rxDataCounter;
    uint8_t txBuffer[BufferSize::TX];
    uint8_t rxBuffer[BufferSize::RX];

  public:
    enum {QUERY = 0xAA, ACK = 0xBB, deviceAddr = 0xDD};
    enum TxDataId
    {
      DATA_ACK = 0,
      SRC_ADDR = 2, //Source Address
      POWER = 4,
      ENERGY = 6
    };
    enum RxDataId
    {
      DATA_QUERY = 0,
      RX_ADDR = 2 //Receiver Address
    };
    HC12(SoftwareSerial* serial,uint8_t setPin,uint32_t baudRate = 9600);
    void EncodeData(uint16_t dataToEncode,TxDataId id);
    void SetChannel(char* channelNum);
    void TransmitData(void);
    bool ReceivedData(void);
    uint16_t DecodeData(RxDataId id);
    void ResetRxBuffer(void);
};
  
