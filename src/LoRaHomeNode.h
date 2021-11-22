#ifndef LORAHOMENODE_H
#define LORAHOMENODE_H

#include <ArduinoJson.h>
#include <Arduino.h>

class LoRaHomeNode
{
public:
    LoRaHomeNode();
    void setup();
    void sendToGateway();
    void receiveLoraMessage();

private:
    void rxMode();
    void txMode();
    bool receiveAck();
    void send(uint8_t* txBuffer, uint8_t size);
    static uint16_t crc16_ccitt(char *data, unsigned int data_len);
};

extern LoRaHomeNode loraHomeNode;

#endif