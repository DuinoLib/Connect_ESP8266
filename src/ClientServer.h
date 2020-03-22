#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <vector>
#include "config.h"
#include <WiFiManager.h>
#include <Base64_AES.h>
/**

**************************Shared Variable***************

*/
// byte *secret_key = (unsigned char *)"12345678123456781234567812345678"; // it should be 16 letter
byte secret_key[] = {239, 121, 124, 129, 24, 240, 45, 251, 100, 150, 7, 221, 93, 63, 140, 118, 35, 4, 140, 156, 6, 61, 83, 44, 201, 92, 94, 215, 168, 152, 166, 79}; //hashlib.sha256(key.encode()).digest()  [from python]
Base64_AES aes(256);

////Readeing data from Client////////

char readeddata[1000]; /// lets make a very long char array

/********************************************************/

/********************************************SERVER*********************************************/

/********************************************CLIENT EVENTS**************************************/

/****************************************DO WHEN SERVER LOCKED******************************************/

/****************************************DO WHEN SERVER IS NOT LOCKED******************************************/

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
    delete client->client_data;// dont forget to delete the previous char array if eisted......
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
  else
  {
    ////////Do something here//////////
    // Serial.println(readeddata);
    size_t expected_msg_len = aes.expected_decrypted_b64_len(client->client_data_len);
    char *decryptedmsg = new char[expected_msg_len];
    aes.decrypt_b64(client->client_data, client->client_data_len, decryptedmsg);
    for (size_t i = 0; i < expected_msg_len; i++)
    {
      readeddata[i] = decryptedmsg[i];
    }
    readeddata[expected_msg_len] = 0; //terminate the last string as 0
    delete decryptedmsg;
    //////////DO SOMETHING HERE//////
    Serial.println("Final msg:");
    Serial.println(readeddata);
    /////////////////////////////////
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
    char message[32];
    sprintf(message, "this is from %s", WiFi.localIP().toString().c_str());
    client->add(message, strlen(message));
    client->send();
  }
}

/* event callbacks */
/*static*/ void Client_handleData(void *arg, AsyncClient *client, void *data, size_t len)
{
  Serial.printf("\n data received from %s \n", client->remoteIP().toString().c_str());
  Serial.write((uint8_t *)data, len);
}

/*static*/ void Client_onConnect(void *arg, AsyncClient *client)
{
  Serial.printf("\n client has been connected to ");
  Client_replyToServer(client);
}

/*static*/ void Client_handleError(void *arg, AsyncClient *client, int8_t error)
{
  Serial.printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}
/*static*/ void Client_handleDisconnect(void *arg, AsyncClient *client)
{
  Serial.printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());
}

/*static*/ void Client_handleTimeOut(void *arg, AsyncClient *client, uint32_t time)
{
  Serial.printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
}

/**

*********************************END CLIENT PART***************************************

*/

/**

**********************************MY FUNCTION***************************************

*/

AsyncClient *client = new AsyncClient;
void connectToServer(char *host, int port)
{
  //  AsyncClient* client = new AsyncClient;
  //  client->onData(&Client_handleData, client);
  //  client->onError(&Client_handleError, NULL);
  //  client->onDisconnect(&Client_handleDisconnect, NULL);
  //  client->onTimeout(&Client_handleTimeOut, NULL);
  //  client->onConnect(&Client_onConnect, client);
  client->connect(host, port);
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
  /******Cleint Set up****/
  client->onData(&Client_handleData, client);
  client->onError(&Client_handleError, NULL);
  client->onDisconnect(&Client_handleDisconnect, NULL);
  client->onTimeout(&Client_handleTimeOut, NULL);
  client->onConnect(&Client_onConnect, client);

  /////////////////////////////////////////////////////////////////

  postsetup();
}