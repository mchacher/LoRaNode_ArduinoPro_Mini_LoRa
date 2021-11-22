#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <LoRaNode.h>
#include <LoRaHomeNode.h>

#define DEBUG

#ifdef DEBUG
#define DEBUG_MSG(x) Serial.println(F(x))
#define DEBUG_MSG_VAR(x) Serial.println(x)
#else
#define DEBUG_MSG(x) // define empty, so macro does nothing
#endif


// sampling management
unsigned long lastSendTime = 0;    // last send time
unsigned long lastProcessTime = 0; // last processing time

void setup()
{

//initialize Serial Monitor
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial)
    ;
#endif
  DEBUG_MSG("initializing LoRa Node");
  // initialize LoRa    
  loraHomeNode.setup();
  // call node specific configuration (end user)
  Node->appSetup();
}

/**
* Main loop of the LoRa Node
* Constantly try to receive JSON LoRa message
* Every transmissionTimeInterval send JSON LoRa messages
*/
void loop()
{
  unsigned long tick = millis();
  if ((tick - lastProcessTime) > Node->getProcessingTimeInterval())
  {
    Node->appProcessing();
    lastProcessTime = millis();
  }
  if (((tick - lastSendTime) > Node->getTransmissionTimeInterval()) || (Node->getTransmissionNowFlag() == true))
  {
    Node->setTransmissionNowFlag(false);
    loraHomeNode.sendToGateway();
    lastSendTime = millis(); // timestamp the message
  }
  loraHomeNode.receiveLoraMessage();
}
