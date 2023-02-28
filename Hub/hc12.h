#pragma once

#define CHANNEL_15 "AT+C015\r\n"

class HC12
{
  private:
    enum BufferSize { TX = 4, RX = 8};
    HardwareSerial* port;
    uint8_t setPin;
    uint8_t rxDataCounter;
    uint8_t txBuffer[BufferSize::TX];
    uint8_t rxBuffer[BufferSize::RX];

  public:
    enum {QUERY = 0xAA, ACK = 0xBB, Node1Addr = 0xCC,
          Node2Addr = 0xDD, Node3Addr = 0xEE};
    enum TxDataId
    {
      DATA_QUERY = 0,
      DEST_ADDR = 2 //Destination address
    };
    enum RxDataId
    {
      DATA_ACK = 0,
      SRC_ADDR = 2,
      POWER = 4,
      ENERGY = 6
    };
    HC12(HardwareSerial* serial,uint8_t Pin,
         uint32_t baudRate = 9600,
         int8_t hc12Tx = -1,
         int8_t hc12Rx = -1);
    void EncodeData(uint16_t dataToEncode,TxDataId id);
    void SetChannel(const char* AT_Cmd);
    void TransmitData(void);
    bool ReceivedData(void);
    uint16_t DecodeData(RxDataId id);
};
  
