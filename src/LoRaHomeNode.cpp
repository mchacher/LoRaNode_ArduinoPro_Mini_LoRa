
#include <LoRaHomeNode.h>
#include <LoRaHomeFrame.h>
#include <LoRa.h>
#include <LoRaNode.h>
#include <ArduinoJson.h>
#include "NodeConfig.h"

#define DEBUG

#ifdef DEBUG
#define DEBUG_MSG(x) Serial.println(F(x))
#define DEBUG_MSG_VAR(x) Serial.println(x)
#else
#define DEBUG_MSG(x) // define empty, so macro does nothing
#define DEBUG_MSG_VAR(x)
#endif

// -------------------------------------------------------
// LoRa HARDWARE CONFIGURATION
// -------------------------------------------------------
//define the pins used by the transceiver module

#ifdef ARDUINO_UNO_BOARD
#define SS (10)
#define RST (5)
#define DIO0 (4)
#endif

// -------------------------------------------------------
// LoRa MODEM SETTINGS
// -------------------------------------------------------
// The sync word assures you don't get LoRa messages from other LoRa transceivers
// ranges from 0-0xFF - make sure that the node is using the same sync word
#define LORA_SYNC_WORD 0xB2
// frequency
// can be changed to 433E6, 915E6
#define LORA_FREQUENCY 868E6
// change the spreading factor of the radio.
// LoRa sends chirp signals, that is the signal frequency moves up or down, and the speed moved is roughly 2**spreading factor.
// Each step up in spreading factor doubles the time on air to transmit the same amount of data.
// Higher spreading factors are more resistant to local noise effects and will be read more reliably at the cost of lower data rate and more congestion.
// Supported values are between 7 and 12
#define LORA_SPREADING_FACTOR 7
// LoRa signal bandwidth
// Bandwidth is the frequency range of the chirp signal used to carry the baseband data.
// Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, and 250E3
#define LORA_SIGNAL_BANDWIDTH 125E3
// Coding rate of the radio
// LoRa modulation also adds a forward error correction (FEC) in every data transmission.
// This implementation is done by encoding 4-bit data with redundancies into 5-bit, 6-bit, 7-bit, or even 8-bit.
// Using this redundancy will allow the LoRa signal to endure short interferences.
// The Coding Rate (CR) value need to be adjusted according to conditions of the channel used for data transmission.
// If there are too many interference in the channel, then it’s recommended to increase the value of CR.
// However, the rise in CR value will also increase the duration for the transmission
// Supported values are between 5 and 8, these correspond to coding rates of 4/5 and 4/8. The coding rate numerator is fixed at 4
#define LORA_CODING_RATE_DENOMINATOR 5

#define ACK_TIMEOUT 2000 // 2000 ms max to receive an Ack
#define MAX_RETRY_NO_VALID_ACK 3


/**
 * @brief Construct a new LoRaHomeNode::LoRaHomeNode object
 * 
 */
LoRaHomeNode::LoRaHomeNode()
{
}
/**
* Set Node in Rx Mode with active invert IQ
* LoraWan principle to avoid node talking to each other
* This way a Gateway only reads messages from Nodes and never reads messages from other Gateway, and Node never reads messages from other Node.
*/
void LoRaHomeNode::rxMode()
{
  LoRa.enableInvertIQ(); // active invert I and Q signals
  LoRa.receive();        // set receive mode
}

/**
* Set Node in Tx Mode with active invert IQ
* LoraWan principle to avoid node talking to each other
* This way a Gateway only reads messages from Nodes and never reads messages from other Gateway, and Node never reads messages from other Node.
*/
void LoRaHomeNode::txMode()
{
  LoRa.idle();            // set standby mode
  LoRa.disableInvertIQ(); // normal mode
}

/**
* initialize LoRa communication with #define settings (pins, SD, bandwidth, coding rate, frequency, sync word)
* CRC is enabled
* set in Rx Mode by default
*/
void LoRaHomeNode::setup()
{
  DEBUG_MSG("LoRaHomeNode::setup");
  //setup LoRa transceiver module
  DEBUG_MSG("--- LoRa Begin");
  LoRa.setPins(SS, RST, DIO0);
  while (!LoRa.begin(LORA_FREQUENCY))
  {
    DEBUG_MSG(".");
    delay(500);
  }
  DEBUG_MSG("--- setSpreadingFactor");
  LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
  DEBUG_MSG("--- setSignalBandwidth");
  LoRa.setSignalBandwidth(LORA_SIGNAL_BANDWIDTH);
  DEBUG_MSG("--- setCodingRate4");
  LoRa.setCodingRate4(LORA_CODING_RATE_DENOMINATOR);
  DEBUG_MSG("--- setSyncWord");
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(LORA_SYNC_WORD);
  DEBUG_MSG("--- enableCrc");
  LoRa.enableCrc();

  // set in rx mode.
  this->rxMode();
}

bool LoRaHomeNode::receiveAck()
{
  unsigned long ackStartWaitingTime = millis();
  //try to parse packet
  int packetSize = LoRa.parsePacket();
  LoRaHomeFrame lhf;
  DEBUG_MSG("LoRaHomeNode::receiveAck");
  // switch to rxMode to receive ACK
  this->rxMode();
  while (((millis() - ackStartWaitingTime) < ACK_TIMEOUT) && (packetSize == 0))
  {
    packetSize = LoRa.parsePacket();
    if (packetSize == LH_FRAME_ACK_SIZE)
    {
      uint8_t rxBuffer[LH_FRAME_ACK_SIZE];
      int j = 0;
      while (packetSize > 0)
      {
        // read available bytes
        rxBuffer[j++] = (char)LoRa.read();
        packetSize--;
      }
      if (lhf.createFromRxMessage(rxBuffer, j, true) == true)
      {
        if ((lhf.nodeIdEmitter == LH_NODE_ID_GATEWAY) && (lhf.nodeIdRecipient == Node->getNodeId()) && (lhf.messageType == LH_MSG_TYPE_GW_ACK))
        {
          if (lhf.counter == Node->getTxCounter())
          {
            DEBUG_MSG("--- good ack received!");
            return true;
          }
        }
      }
      else
      {
        packetSize = 0; // to loop again
        DEBUG_MSG("--- bad ack received!");
      }
    }
  }
  DEBUG_MSG("--- no ACK received");
  return false;
}

/**
* [sendToLora2MQTTGateway description]
*/
void LoRaHomeNode::sendToGateway()
{
  int retry = 0;
  DEBUG_MSG("LoRaHomeNode::sendToGateway()");
  uint8_t txBuffer[LH_FRAME_MAX_SIZE];
  DEBUG_MSG("--- create LoraHomeFrame");
  // create frame
  LoRaHomeFrame lhf(MY_NETWORK_ID, Node->getNodeId(), LH_NODE_ID_GATEWAY, LH_MSG_TYPE_NODE_MSG_ACK_REQ, Node->getTxCounter());
  // create payload
  DEBUG_MSG("--- create LoraHomePayload");
  StaticJsonDocument<128> jsonDoc;
  Node->addJsonTxPayload(jsonDoc);
  serializeJson(jsonDoc, lhf.jsonPayload, LH_FRAME_MAX_PAYLOAD_SIZE);
  //add payload to the frame if any
  uint8_t size = lhf.serialize(txBuffer);
  DEBUG_MSG("--- LoraHomeFrame serialized");
  // send the LoRa message until valid ack is received with max retries
  do
  {
    retry++;
    this->send(txBuffer, size);
  } while ((receiveAck() == false) && (retry < MAX_RETRY_NO_VALID_ACK));
  // increment TxCounter
  // TODO should only increment TxCounter if msg sent + ack received ... else error
  Node->incrementTxCounter();
}

/**
 * @brief 
 * 
 * @param txBuffer 
 */
void LoRaHomeNode::send(uint8_t *txBuffer, uint8_t size)
{
  DEBUG_MSG("LoRaHomeNode::send");
  DEBUG_MSG("--- sending LoRa message to LoRa2MQTT gateway");
  this->txMode();
  LoRa.beginPacket();
  for (uint8_t i = 0; i < size; i++)
  {
    LoRa.write(txBuffer[i]);
    //DEBUG_MSG_VAR(txBuffer[i]);
  }
  LoRa.endPacket();
  this->rxMode();
}

/**
* [receiveLoraMessage description]
*/
void LoRaHomeNode::receiveLoraMessage()
{
  //try to parse packet
  int packetSize = LoRa.parsePacket();
  // return immediately if no message available
  if (packetSize == 0)
  {
    return;
  }
  // check if we can accept the message
  if ((packetSize > LH_FRAME_MAX_SIZE) || (packetSize < LH_FRAME_MIN_SIZE))
  {
    while (LoRa.available())
    {
      // flush Fifo
      LoRa.read();
    }
    return;
  }
  DEBUG_MSG("LoRaHomeNode::receiveLoraMessage");
  // read bytes available
  uint8_t rxMessage[LH_FRAME_MAX_SIZE];
  uint8_t j = 0;
  for (j = 0; j < packetSize; j++)
  {
    // read available bytes
    rxMessage[j] = (char)LoRa.read();
  }
  // create LoRa Home frame
  LoRaHomeFrame lhf;
  lhf.createFromRxMessage(rxMessage, j, true);
  if (lhf.networkID != MY_NETWORK_ID)
  {
    DEBUG_MSG("--- ignore message, not the right network ID");
    return;
  }
  // if (lhf.messageType != LH_MSG_TYPE_GW_MSG_ACK)
  // {
  //   DEBUG_MSG("--- ignore message, not a gateway message with Ack");
  //   return;
  // }
  // No error we can process the message
  DEBUG_MSG("--- message received");
  // serializeJson(jsonDoc, Serial);
  uint8_t nodeInvoked = lhf.nodeIdRecipient;
  // Am I the node invoked for this messages
  if (nodeInvoked == Node->getNodeId())
  {
    // I am the one!
    DEBUG_MSG("--- I am node invoked");
    // parse JSON message
    StaticJsonDocument<LH_FRAME_MAX_PAYLOAD_SIZE> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, lhf.jsonPayload);
    // deserializeJson error
    if (error)
    {
      DEBUG_MSG("--- deserializeJson error");
      return;
    }
    // if message received request an ack
    if ((lhf.messageType == LH_MSG_TYPE_GW_MSG_ACK) || (lhf.messageType == LH_MSG_TYPE_NODE_MSG_ACK_REQ))
    {
      LoRaHomeFrame lhfAck(MY_NETWORK_ID, Node->getNodeId(), lhf.nodeIdEmitter, LH_MSG_TYPE_NODE_ACK, lhf.counter);
      uint8_t txBuffer[LH_FRAME_MIN_SIZE];
      uint8_t size = lhfAck.serialize(txBuffer);
      this->send(txBuffer, size);
      DEBUG_MSG("--- ack sent");
    }
    //JsonObject root = jsonDoc.to<JsonObject>();
    Node->parseJsonRxPayload(jsonDoc);
  }
}

LoRaHomeNode loraHomeNode;