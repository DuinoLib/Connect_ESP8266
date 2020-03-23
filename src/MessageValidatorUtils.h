#ifndef MESSAGE_VALIDATOR_UTILS_H
#define MESSAGE_VALIDATOR_UTILS_H

#include <ESP8266WiFi.h>

////////////////////////////////DEFINE USER CHOICE////////////////////
#define UNIQUE_LEN 16             //this will be the length of the salt or msessage id
#define MAX_PENDING_TIME 10000000 //10 seconds in microseconds

boolean compareSalt(const char *salt1, char *salt2, unsigned long salt_time, unsigned long curmicro)
{
    if (curmicro - salt_time > MAX_PENDING_TIME)
    {
        return false;
    }
    for (size_t i = 0; i < UNIQUE_LEN; i++) // interating over the char of char array to check if the char array are same
    {
        if (salt1[i] != salt2[i])
        {
            LOG("Not Salt matches");
            LOGF(salt1, salt2);
            return false;
        }
    }
    VER("Salt matches");
    VERF(salt1, salt2);
    return true;
}

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