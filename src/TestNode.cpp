#include <LoRaNode.h>
#include <Arduino.h>
#include <TestNode.h>
#include "NodeConfig.h"

#define DEBUG
#ifdef DEBUG
#define DEBUG_MSG(x) Serial.println(F(x))
#define DEBUG_MSG_VAR(x) Serial.println(x)
#else
#define DEBUG_MSG(x) // define empty, so macro does nothing
#define DEBUG_MSG_VAR(x)
#endif


/**
 * @brief Construct a new Reed Switch Node:: Reed Switch Node object
 * 
 */
TestNode::TestNode()
{
    // empty
}

/**
* Function invoked by the node right after its own setup (as per Arduino Setup function)
* To be used for applicative setup
*/
void TestNode::appSetup()
{
    // set the Id of the node
    this->setNodeId(NODE_ID);
    this->setProcessingTimeInterval(PROCESSING_TIME_INTERVAL);
    this->setTransmissionTimeInterval(TRANSMISSION_TIME_INTERVAL);
    // ask for current state transmission
    LoRaNode::setTransmissionNowFlag(true);
}


/**
* Add JSON Tx payload messages
* @param payload the JSON payload to be completed as per application needs
*/
void TestNode::addJsonTxPayload(JsonDocument &payload)
{
    // send a simple tx counter
    static uint8_t i = 0;
    payload["tx"] = i++;
    DEBUG_MSG("--- Send msg ...");
}

/**
* Parse JSON Rx payload
* One should avoid any long processing in this routine. LoraNode::AppProcessing is the one to be used for this purpose
* Limit the processing to parsing the payload and retrieving the expected attributes
* @param payload the JSON payload received by the node
*/
void TestNode::parseJsonRxPayload(JsonDocument &payload)
{
   // assume receicing a message with json key "msg", display it
   if (payload["msg"].isNull() == false)
    {
        DEBUG_MSG("--- receive:");
        DEBUG_MSG_VAR((const char*)payload["msg"]);
    }
}

/**
* App processing of the node.
* Invoke every loop of the nodes before Rx and Tx
* ONe should benefit from using processingTimeInterval to avoid overloading the node
*/
void TestNode::appProcessing()
{
    // nothing
}

LoRaNode *Node = new TestNode();