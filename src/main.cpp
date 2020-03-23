#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ClientServer.h>
#include <MyDebug.h>

uint16_t TCP_PORT = 8080;

void presetup()
{
  Serial.begin(115200);
  Serial.println();
  uint32_t free_ram = system_get_free_heap_size();
  VERF("Free ram at init:",free_ram);
}

void postsetup() {

}

unsigned long mili = 0;
void loop()
{
  if (millis() - mili > 2000)
  {
    mili = millis();
    uint32_t free = system_get_free_heap_size();
    VERF("Free Heap:",free);
  }
}