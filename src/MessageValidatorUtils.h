#ifndef MESSAGE_VALIDATOR_UTILS_H
#define MESSAGE_VALIDATOR_UTILS_H

#include <ESP8266WiFi.h>
void getrandom(char *ran, size_t len)
{
    // we are trying to generate only alpha numerical salt
    uint8_t exclusions[]{91, 92, 93, 94, 95, 96};
    for (size_t i = 0; i < len - 1; i++)
    {
        uint8_t val;
        do
        {
            val = random(65, 123);
        } while ([&exclusions, &val]() {
            for (uint8_t k : exclusions)
            {
                if (val == k)
                {
                    return true;
                }
            }
            return false;
        }());
        ran[i] = val;
    }
    ran[len - 1] = 0;
}

#endif