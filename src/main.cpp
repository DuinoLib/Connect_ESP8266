#include <Arduino.h>
#include <ClientServer.h>

uint16_t TCP_PORT = 8080;

void presetup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.print("\nFree Ram at presetup:");
  uint32_t free = system_get_free_heap_size();
  Serial.print(free);
}

void postsetup() {}

/////////////////////
char host[] = "192.168.29.242";
int port = 12345;

unsigned long mili = 0;
void loop()
{
  if (millis() - mili > 2000)
  {
    mili = millis();
    uint32_t free = system_get_free_heap_size();
    Serial.print("\nFree Ram:");
    Serial.print(free);
    // connectToServer(host, port);
  }
}