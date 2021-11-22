#ifndef LORANODE_H
#define LORANODE_H

#include <ArduinoJson.h>
#include <Arduino.h>

#define ARDUINO_UNO_BOARD

class LoRaNode
{
public:
  LoRaNode();

  /**
    * Setup of the node.
    * Invoke at startup
    */
  virtual void appSetup() = 0;
  /**
    * App processing of the node.
    * Invoke every processing time interval of the nodes before Rx and Tx
    * One should benefit from well defining processingTimeInterval to avoid overloading the node
    */
  virtual void appProcessing() = 0;
  /**
    * Add JSON Tx payload messages
    * @param payload the JSON payload to be completed as per application needs
    */
  virtual void addJsonTxPayload(JsonDocument &payload) = 0;
  /**
    * Parse JSON Rx payload
    * One should avoid any long processing in this routine. LoraNode::AppProcessing is the one to be used for this purpose
    * Limit the processing to parsing the payload and retrieving the expected attributes
    * @param payload the JSON payload received by the node
    */
  virtual void parseJsonRxPayload(JsonDocument &payload) = 0;


  void setNodeId(uint8_t nodeId);
  uint8_t getNodeId();
  unsigned long getTransmissionTimeInterval();
  unsigned long getProcessingTimeInterval();
  void setTransmissionTimeInterval(unsigned long timeInterval);
  void setProcessingTimeInterval(unsigned long timeInterval);
  uint16_t getTxCounter();
  void incrementTxCounter();
  static void setTransmissionNowFlag(bool flag);
  static bool getTransmissionNowFlag();

private:
  uint8_t NodeId = 0;
  uint16_t TxCounter = 0;
  unsigned long transmissionTimeInterval = 10000; // 10
  // node processing time interval
  unsigned long processingTimeInterval = 180000;
  // to force immediate transmission
  volatile static bool needTransmissionNow;
};

extern LoRaNode *Node;

#endif