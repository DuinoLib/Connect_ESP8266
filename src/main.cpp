#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ClientServer.h>
#include <MyDebug.h>

#define LED_BUILT 2
//////////////////////////////////////////
uint16_t TCP_PORT = 8080; ///change this server port if you want

////////////////////Condtant for LED//////////////
bool LED_STATE = false;
////////////////////////////////////////////////
//////Never use delay;except in presetup() and postsetup;
void presetup()
{
  Serial.begin(115200);
  Serial.println();
  uint32_t free_ram = system_get_free_heap_size();
  SRLF("Free ram at init:", free_ram);

  /////////////////LED///////////////
  pinMode(LED_BUILT, OUTPUT);

  LED_STATE = !LED_STATE;
  digitalWrite(BUILTIN_LED, LED_STATE);
  delay(2000);
  LED_STATE = !LED_STATE;
  digitalWrite(BUILTIN_LED, LED_STATE);
  delay(2000);
  LED_STATE = !LED_STATE;
  digitalWrite(BUILTIN_LED, LED_STATE);
  delay(2000);
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
    SRLF("Free Heap", free);
  }

  if (millis() - mili2 > 2000)
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

//////////////////we do the folowing task when valid message is recieved/////////

void PerformTask(const JsonDocument &json)
{
  // doc["tag"] doc["taskName"] doc["message"] doc["extra"]
  if (compare("Hello", (const char *)json["tag"], 5))
  {
    LED_STATE = !LED_STATE;
    digitalWrite(BUILTIN_LED, LED_STATE);
  }
}
