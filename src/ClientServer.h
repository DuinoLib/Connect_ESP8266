#ifndef CLIENTSERVER_H
#define CLIENTSERVER_H

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <vector>
#include "config.h"
#include <WiFiManager.h>
#include <Base64_AES.h>
#include <ArduinoJson.h>
#include <MessageValidator.h>
#include <MyDebug.h>
/**

**************************Shared Variable***************

*/
// byte *secret_key = (unsigned char *)"12345678123456781234567812345678"; // it should be 16 letter
byte secret_key[] = {239, 121, 124, 129, 24, 240, 45, 251, 100, 150, 7, 221, 93, 63, 140, 118, 35, 4, 140, 156, 6, 61, 83, 44, 201, 92, 94, 215, 168, 152, 166, 79}; //hashlib.sha256(key.encode()).digest()  [from python]
Base64_AES aes(256);

/********************************************************/

/******************************************* AS A SERVER PART *********************************************/

/******************************************** CLIENT EVENTS **************************************/

/*static*/ void Server_handleError(void *arg, AsyncClient *client, int8_t error)
{
  LOG("Server has got error conection with a client");
  ////delete the client's readed data properly////
  ////we will nothing to do with this client so disconnect and delete it properly////
  if (client)
  {
    if (client->is_new_data)
    {
      client->is_new_data = false;
      client->client_data_len = 0;
      delete client->client_data;
    }
    client->close(true);
    delete client;
  }
}

/*static*/ void Server_handleDisconnect(void *arg, AsyncClient *client)
{
  LOG("Server handling disconnection of a client");
  ///////////////////Lets handele the message after the disconnection/////////////////////
  size_t expected_msg_len = aes.expected_decrypted_b64_len(client->client_data_len);
  char *decryptedmsg = new char[expected_msg_len + 1];
  aes.decrypt_b64(client->client_data, client->client_data_len, decryptedmsg);
  decryptedmsg[expected_msg_len] = 0; /// lets end with zero}
  LOG(decryptedmsg);
  //////////DO SOMETHING HERE//////
  validate(decryptedmsg, expected_msg_len); // lets ctake appropate action for the message
  /////////////////////////////////
  delete decryptedmsg;
  //////////////////////////////////////////////////////////////////////
  ////we will nothing to do with this client so disconnect and delete it properly////
  if (client)
  {
    if (client->is_new_data)
    {
      client->is_new_data = false;
      client->client_data_len = 0;
      delete client->client_data;
    }
    client->close(true);
    delete client;
  }
}

/*static*/ void Server_handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
  LOG("Server time ACK timeout at a client");
  ////we will nothing to do with this client so disconnect and delete it properly////
  ////we will nothing to do with this client so disconnect and delete it properly////
  if (client)
  {
    if (client->is_new_data)
    {
      client->is_new_data = false;
      client->client_data_len = 0;
      delete client->client_data;
    }
    client->close(true);
    delete client;
  }
}

/*static*/ void Server_handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
  LOG("Server is receiving data from a client");
  ////////////////////////////////////////
  /*
  *
  *Note: this function is called more than once if the message sent by the client is too big
  *
  */

  ///////this follwowing is appending the mesage to the client object's "client_data"
  size_t new_data_len = client->client_data_len + len;
  char *new_data = new char[new_data_len];
  for (size_t i = 0; i < client->client_data_len; i++)
  {
    new_data[i] = client->client_data[i];
  }
  for (size_t i = 0; i < len; i++)
  {
    new_data[client->client_data_len + i] = ((char *)data)[i];
  }
  client->client_data_len = new_data_len;
  if (client->is_new_data)
  {
    delete client->client_data; // dont forget to delete the previous char array if existed......
  }
  client->client_data = new char[new_data_len];
  client->is_new_data = true; // set ther is new data in the client object
  for (size_t i = 0; i < new_data_len; i++)
  {
    client->client_data[i] = new_data[i];
  }
  delete new_data;
  ///////////////////////end of appending the message to client data in cleint data object///////////////
  /**
   * 
   * As this function is call more than once we should not handle the message sent by the client here.
   * we will handle it in the "Server_handleDisconnect()"
   * 
   */
  if (client->space() > 32 && client->canSend())
  {
    ////lets send some empty space message so that the client can also disconnect by it own
    char reply[] = " ";
    client->add(reply, strlen(reply));
    client->send();
  }
}
/*************************************WHEN NEW CONNECTION***************************/
/*static*/ void handleNewClient(void *arg, AsyncClient *client)
{
  LOG("Server is connected by a cleint");
  client->onData(&Server_handleData, NULL);
  client->onError(&Server_handleError, NULL);
  client->onDisconnect(&Server_handleDisconnect, NULL);
  client->onTimeout(&Server_handleTimeOut, NULL);
}

/**
 *  
 * *******************************END SERVER PART***************************************
 * 
 */

/**
 * 
 * ************************************ AS A CLIENT PART *****************************************
 * 
 */

/*static*/ void Client_replyToServer(void *arg)
{
  AsyncClient *client = reinterpret_cast<AsyncClient *>(arg);
  if (client->space() > 32 && client->canSend())
  {
    LOG("Sending encrypted mesage to a TCP server");
    client->add(client->client_data, client->client_data_len);
    client->send();
  }
}

/* event callbacks */
/*static*/ void Client_handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
  //we are not interested with ata recieved from the server so do nothing
  LOG("Recieving useless message from a TCP server.");
}

/*static*/ void Client_onConnect(void *arg, AsyncClient *client)
{
  LOG("Has been connected to a TCP server");
  Client_replyToServer(client);
}

/*static*/ void Client_handleError(void *arg, AsyncClient *client, int8_t error)
{
  LOG("Got connection error in connecting to a TCP server");
  ////we will nothing to do with this client so disconnect and delete it properly////
  ////we will nothing to do with this client so disconnect and delete it properly////
  if (client)
  {
    if (client->is_new_data)
    {
      client->is_new_data = false;
      client->client_data_len = 0;
      delete client->client_data;
    }
    client->close(true);
    delete client;
  }
}
/*static*/ void Client_handleDisconnect(void *arg, AsyncClient *client)
{
  LOG("Handling disconnection from a TCP server");
  ////we will nothing to do with this client so disconnect and delete it properly////
  if (client)
  {
    if (client->is_new_data)
    {
      client->is_new_data = false;
      client->client_data_len = 0;
      delete client->client_data;
    }
    client->close(true);
    delete client;
  }
}

/*static*/ void Client_handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
  LOG("Time out in a connection to a TCP server");
  ////we will nothing to do with this client so disconnect and delete it properly////
  if (client)
  {
    if (client->is_new_data)
    {
      client->is_new_data = false;
      client->client_data_len = 0;
      delete client->client_data;
    }
    client->close(true);
    delete client;
  }
}

/**
 * 
 *********************************END OF AS ACLIENT PART***************************************
 *
 */

/**
 * 
 * **********************************MY FUNCTION***************************************
 * 
 */

void connectToServer(const char *host, int port, char *msg, size_t len)
{
  AsyncClient *client = new AsyncClient;
  client->connect(host, port);

  ///////////////////////////////////////

  client->onData(&Client_handleData, client);
  client->onError(&Client_handleError, NULL);
  client->onDisconnect(&Client_handleDisconnect, NULL);
  client->onTimeout(&Client_handleTimeOut, NULL);
  client->onConnect(&Client_onConnect, client);
  /******Cleint Set up****/
  client->client_data = msg;
  client->client_data_len = len;
  client->is_new_data = true;
}

/***********************************INTERFCE******************************************/
void presetup();
void postsetup();
/**************************************************************************************/


/**
 * 
 ************************SEND A FRESH MESSAGE************************** 
 * 
 */
void sendMessage(
    char *receiverIp,
    uint16_t recieverPort,
    size_t tag_len,
    size_t taskName_len,
    char *tag ,
    char *taskName,
    size_t message_len=0,
    char *message=(char*)NULL,
    size_t extra_len = 0,
    char *extra = (char*)NULL,
    int8_t mtype = TYPE_MESSAGE,
    bool isIntent = false,
    int8_t resultcode = RESULT_UNKNOWN)
{

  /////generate a valid salt/////

  /// lets send init message/////

  int init_index = generateMsg();
  Message *msg = myMsgs[init_index];
  msg->set(taskName_len,
          message_len,
          taskName,
          message,
          tag_len,
          tag,
          extra_len,
          extra,
          mtype,
          isIntent,
          resultcode);
  size_t salt_index = generateSalt();
  
  ///// Lets construct init message///////
  StaticJsonDocument<JSON_SIZE> constructed_init_msg;
  constructMessage(
      constructed_init_msg,
      TYPE_INIT,
      salt_index,
      init_index,
      false,
      NULL,
      NULL,
      NULL,
      NULL,
      COMMUTYPE_WIFI,
      RESULT_UNKNOWN);
  int sz = measureJson(constructed_init_msg) + 2;
  char *msg2send = new char[sz];
  serializeJson(constructed_init_msg, msg2send, sz);
  msg2send[sz - 1] = 0; //terminating the string with zero in case....
  DEBUGF("RAW Messages to send", msg2send);
  handleEncryptedsend(msg2send,sz,receiverIp,recieverPort);
  delete msg2send;
}
//////////////////////////////////////////////////////////////////////////////
extern uint16_t TCP_PORT; //this port is where this device will open a TCP server

AsyncServer *server = new AsyncServer(TCP_PORT); // start listening on tcp port 7050

void setup()
{
  presetup();
  aes.setkey(secret_key); //Never forget to set the key
  ////////////////////////////////////////////////////////////////
  WiFiManager wifiManager;
  //  wifiManager.setDebugOutput(false);
  wifiManager.autoConnect("NodeMcu_EspWifi", "1234506789");
  //////////////////////////////////////////////////////////////////
  /******Server Set up****/
  //  AsyncServer* server = new AsyncServer(TCP_PORT); // start listening on tcp port 7050
  server->onClient(&handleNewClient, server);
  server->begin();
  /////////////////////////////////////////////////////////////////
  postsetup();
}

#endif