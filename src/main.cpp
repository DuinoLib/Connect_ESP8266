#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ClientServer.h>
#include <MyDebug.h>

uint16_t TCP_PORT = 8080;
///////////////////////////
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
    int8_t resultcode = RESULT_UNKNOWN);
//////////////////////////

void presetup()
{
  Serial.begin(115200);
  Serial.println();
  uint32_t free_ram = system_get_free_heap_size();
  VERF("Free ram at init:", free_ram);
}

void postsetup()
{
}

unsigned long mili1 = 0;
unsigned long mili2 = 0;
void loop()
{
  if (millis() - mili1 > 2000)
  {
    mili1 = millis();
    uint32_t free = system_get_free_heap_size();
    VERF("Free Heap", free);
  }

  if (millis() - mili2 > 5000)
  {
    mili2 = millis();
    char *receiverIP = "192.168.29.242";
    uint16_t receiverPort = 6868;
    char *tag = "Hodie jjj";
    size_t tag_len = strnlen(tag, 20);
    char *task_name = "My message";
    size_t taskname_len = strnlen(task_name, 20);
    sendMessage(
        receiverIP,
        receiverPort,
        tag_len,
        taskname_len,
        tag,
        task_name);
  }
}

///////////////////////////////////////////////////////////
void sendMessage(
    char *receiverIp,
    uint16_t recieverPort,
    size_t tag_len,
    size_t taskName_len,
    char *tag ,
    char *taskName,
    size_t message_len,
    char *message,
    size_t extra_len,
    char *extra ,
    int8_t mtype,
    bool isIntent ,
    int8_t resultcode)
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