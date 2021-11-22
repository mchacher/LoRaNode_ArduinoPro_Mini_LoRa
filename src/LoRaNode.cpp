#include <LoRaNode.h>
#include <Arduino.h>

#define DEBUG

#ifdef DEBUG
#define DEBUG_MSG(x) Serial.println(F(x))
#define DEBUG_MSG_VAR(x) Serial.println(x)
#else
#define DEBUG_MSG(x) // define empty, so macro does nothing
#endif

volatile bool LoRaNode::needTransmissionNow = false;

/**
* LoRaNode Constructor. Empty
*/
LoRaNode::LoRaNode()
{
}

/**
 * @brief Get the ID of the Node
 * 
 * @return uint8_t the Node Id
 */
uint8_t LoRaNode::getNodeId()
{
  return this->NodeId;
}

/**
 * @brief set the Node Id
 * 
 * @param nodeId the node Id
 */
void LoRaNode::setNodeId(uint8_t nodeId)
{
  this->NodeId = nodeId;
}

/**
* Get transmission time interval
* @return transmission time interval in ms. User defined parameter.
*/
unsigned long LoRaNode::getTransmissionTimeInterval()
{
  return this->transmissionTimeInterval;
}

/**
 * @brief Set the ProcessingTimeInterval to callback AppProcessing method of the node
 * 
 * @param TimeInterval value in ms
 */
void LoRaNode::setProcessingTimeInterval(unsigned long timeInterval)
{
  this->processingTimeInterval = timeInterval;
}

/**
 * @brief Set the Trnasmission time interval of the Node 
 * 
 * @param TimeInterval value in ms
 */
void LoRaNode::setTransmissionTimeInterval(unsigned long timeInterval)
{
  this->transmissionTimeInterval = timeInterval;
}

/**
 * @brief get the TxCounter value of the Node
 * 
 * @return uint16_t TxCounter of the Node 
 */
uint16_t LoRaNode::getTxCounter()
{
  return this->TxCounter;
}

void LoRaNode::incrementTxCounter()
{
  this->TxCounter++;
}

/**
* Get processing time interval
* @return processing time interval in ms. User defined paramater.
*/
unsigned long LoRaNode::getProcessingTimeInterval()
{
  return processingTimeInterval;
}

bool LoRaNode::getTransmissionNowFlag()
{
  return needTransmissionNow;
}

void LoRaNode::setTransmissionNowFlag(bool flag)
{
  needTransmissionNow = flag;
}
