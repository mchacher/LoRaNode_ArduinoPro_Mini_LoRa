#ifndef FOURRELAYSNODE_H
#define FOURRELAYSNODE_H

#include <ArduinoJson.h>
#include <Arduino.h>
#include <LoRaNode.h>

class TestNode: public LoRaNode
{
    public : 
        TestNode();
        void appSetup();
        void appProcessing();
        void addJsonTxPayload(JsonDocument &payload);
        void parseJsonRxPayload(JsonDocument &payload);
    private : 
        void commandRelay(uint8_t relay, uint8_t cmd);
        void parseJsonRelayCmd(uint8_t relay, char* cmd);
};

#endif