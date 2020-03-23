#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <vector>
#include "config.h"
#include <WiFiManager.h>
#include <Base64_AES.h>
#include <ArduinoJson.h>
#include <MessageValidator.h>
/**

**************************Shared Variable***************

*/
// byte *secret_key = (unsigned char *)"12345678123456781234567812345678"; // it should be 16 letter
byte secret_key[] = {239, 121, 124, 129, 24, 240, 45, 251, 100, 150, 7, 221, 93, 63, 140, 118, 35, 4, 140, 156, 6, 61, 83, 44, 201, 92, 94, 215, 168, 152, 166, 79}; //hashlib.sha256(key.encode()).digest()  [from python]
Base64_AES aes(256);

/********************************************************/

/********************************************SERVER*********************************************/

/********************************************CLIENT EVENTS**************************************/

/*static*/ void Server_handleError(void *arg, AsyncClient *client, int8_t error)
{
  Serial.printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
  ////delete the client's readed data properly////
  if (client->is_new_data)
  {
    client->is_new_data = false;
    delete client->client_data;
  }
  ///////////////
  client->close(true);
  client->free();
  delete client;
}

/*static*/ void Server_handleDisconnect(void *arg, AsyncClient *client)
{
  Serial.printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());

  ///////////////////LETS DECRYPT WHEN DISCONNECTED/////////////////////
  ////////Do something here//////////
  size_t expected_msg_len = aes.expected_decrypted_b64_len(client->client_data_len);
  char *decryptedmsg = new char[expected_msg_len + 1];

  aes.decrypt_b64(client->client_data, client->client_data_len, decryptedmsg);
  // size_t strlen_d = strlen(decryptedmsg);
  // if (strlen_d < expected_msg_len)
  // {
  //   decryptedmsg[strlen_d] = 0; /// lets end with zero}
  // }
  // else
  // {
  //   decryptedmsg[expected_msg_len] = 0; /// lets end with zero}
  // }
  decryptedmsg[expected_msg_len] = 0; /// lets end with zero}
  //////////DO SOMETHING HERE//////
  validate(decryptedmsg, expected_msg_len);
  /////////////////////////////////
  delete decryptedmsg;

  //////////////////////////////////////////////////////////////////////
  ////delete the client's readed data properly////
  if (client->is_new_data)
  {
    client->is_new_data = false;
    delete client->client_data;
  }
  ///////////////
  client->close(true);
  client->free();
  delete client;
}

/*static*/ void Server_handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
  Serial.printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
  Serial.println();
  ////delete the client's readed data properly////
  if (client->is_new_data)
  {
    client->is_new_data = false;
    delete client->client_data;
  }
  ///////////////
  client->close(true);
  client->free();
  delete client;
}

/*static*/ void Server_handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
  Serial.printf("\n data received from client %s \n", client->remoteIP().toString().c_str());
  ////////////////////////////////////////

  // Serial.println((char*)data);
  //////////THIS IS JUST APPENDING "data" to "client_data"////////////
  size_t new_data_len = client->client_data_len + len;
  char *new_data = new char[new_data_len];
  for (size_t i = 0; i < client->client_data_len; i++)
  {
    new_data[i] = client->client_data[i];
  }
  for (size_t i = 0; i < len; i++)
  {
    new_data[client->client_data_len + i] = ((char *)data)[i];
    // Serial.println(((char *)data)[i]);
  }
  client->client_data_len = new_data_len;
  if (client->is_new_data)
  {
    delete client->client_data; // dont forget to delete the previous char array if existed......
  }
  client->client_data = new char[new_data_len];
  client->is_new_data = true;
  for (size_t i = 0; i < new_data_len; i++)
  {
    client->client_data[i] = new_data[i];
  }
  delete new_data;
  //////////////////////THIS WAS JUST APPENDING "data" to "client_data"/////////////////////////

  if (client->space() > 32 && client->canSend())
  {
    ///////////////////SEND DATA IF YOU WANT//////////////////////
    char reply[] = " ";
    client->add(reply, strlen(reply));
    client->send();
  }
}
/*************************************WHEN NEW CONNECTION***************************/
/*static*/ void handleNewClient(void *arg, AsyncClient *client)
{
  Serial.printf("\n new client has been connected to server, ip: %s", client->remoteIP().toString().c_str());
  client->onData(&Server_handleData, NULL);
  client->onError(&Server_handleError, NULL);
  client->onDisconnect(&Server_handleDisconnect, NULL);
  client->onTimeout(&Server_handleTimeOut, NULL);
}

/**

*********************************END SERVER PART***************************************

*/

/**

************************************CLIENT PART*****************************************

*/

/*static*/ void Client_replyToServer(void *arg)
{
  AsyncClient *client = reinterpret_cast<AsyncClient *>(arg);

  // send reply
  if (client->space() > 32 && client->canSend())
  {
    // Serial.println("\nEncrypted content TOBESEND:");
    // Serial.println(client->client_data);
    client->add(client->client_data, client->client_data_len);
    client->send();
  }
}

/* event callbacks */
/*static*/ void Client_handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
  Serial.println("Dta Recieve");
}

/*static*/ void Client_onConnect(void *arg, AsyncClient *client)
{
  Serial.printf("\n client has been connected to ");
  Client_replyToServer(client);
}

/*static*/ void Client_handleError(void *arg, AsyncClient *client, int8_t error)
{
  Serial.printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
  if (client->is_new_data)
  {
    client->is_new_data = false;
    client->client_data_len = 0;
    delete client->client_data;
  }
  ///////////////
  client->close(true);
  client->free();
  delete client;
}
/*static*/ void Client_handleDisconnect(void *arg, AsyncClient *client)
{
  Serial.printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());
  if (client->is_new_data)
  {
    client->is_new_data = false;
    client->client_data_len = 0;
    delete client->client_data;
  }
  ///////////////
  client->close(true);
  client->free();
  delete client;
}

/*static*/ void Client_handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
  if (client->is_new_data)
  {
    Serial.printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
    client->is_new_data = false;
    client->client_data_len = 0;
    delete client->client_data;
  }
  ///////////////
  client->close(true);
  client->free();
  delete client;
}

/**

*********************************END CLIENT PART***************************************

*/

/**

**********************************MY FUNCTION***************************************

*/

void connectToServer(const char *host, int port, char *msg, size_t len)
{
  //  AsyncClient* client = new AsyncClient;
  //  client->onData(&Client_handleData, client);
  //  client->onError(&Client_handleError, NULL);
  //  client->onDisconnect(&Client_handleDisconnect, NULL);
  //  client->onTimeout(&Client_handleTimeOut, NULL);
  //  client->onConnect(&Client_onConnect, client);

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

extern uint16_t TCP_PORT;

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