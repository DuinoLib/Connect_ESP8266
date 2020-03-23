
#ifndef MYDEBUG_H
#define MYDEBUG_H


// #define DEBUG_ESP_PORT
// #define LOG_ESP
#define VER_ESP

#ifdef DEBUG_ESP_PORT
#define DEBUG(...) Serial.print("Debug--> ");Serial.println(__VA_ARGS__)
#define DEBUGF(sname,...) Serial.print("Debug--> ");Serial.print(sname);Serial.print(": ");Serial.println(__VA_ARGS__)
#else
#define DEBUG(...)
#define DEBUGF(...)
#endif

#ifdef LOG_ESP
#define LOG(...) Serial.print("Log--> ");Serial.println(__VA_ARGS__)
#define LOGF(sname,...) Serial.print("Log--> ");Serial.print(sname);Serial.print(": ");Serial.println(__VA_ARGS__)
#else
#define LOG(...)
#define LOGF(...)
#endif

#ifdef VER_ESP
#define VER(...) Serial.print("Log--> ");Serial.println(__VA_ARGS__)
#define VERF(sname,...) Serial.print("Log--> ");Serial.print(sname);Serial.print(": ");Serial.println(__VA_ARGS__)
#else
#define VER(...)
#define VERF(...)
#endif

#endif