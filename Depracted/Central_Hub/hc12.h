#pragma once
 
class HC12
{
  private:
    enum BufferSize {TX = 9, RX = 5};
    HardwareSerial* port;
    uint8_t rxDataCounter;
    uint8_t txBuffer[BufferSize::TX]; 
    uint8_t rxBuffer[BufferSize::RX];
    
  public:
    enum {QUERY = 0xAA, ACK = 0xBB};
    enum TxDataId 
    {

    };
    enum RxDataId
    {

    };
    
    HC12(HardwareSerial* serial,
         uint32_t baudRate = 9600,
         int8_t hc12Tx = -1,
         int8_t hc12Rx = -1);
    /*Transmitter*/
    void EncodeData(uint8_t dataToEncode,TxDataId id);
    void TransmitData(void);
    /*Non-blocking Receiver*/
    bool ReceivedData(void);
    uint8_t DecodeData(RxDataId id);
};
