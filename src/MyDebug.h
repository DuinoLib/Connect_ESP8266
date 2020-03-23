
#ifndef MYDEBUG_H
#define MYDEBUG_H
///DEFFINE//////
// #define DEBUG_ESP
// #define LOG_ESP
#define VER_ESP
#define ERR_ESP
///////////////

unsigned long milli=0;

#ifdef DEBUG_ESP
#define DEBUG(...)             \
    Serial.print("Debug--> "); \
    Serial.print(__VA_ARGS__);\
    Serial.print("[");\
    Serial.print(millis()-milli);\
    Serial.println("]");\
    milli=millis()
    
#define DEBUGF(sname, ...)     \
    Serial.print("Debug--> "); \
    Serial.print(sname);       \
    Serial.print(": ");        \
    Serial.print(__VA_ARGS__);\
    Serial.print("[");\
    Serial.print(millis()-milli);\
    Serial.println("]");\
    milli=millis()
#else
#define DEBUG(...)
#define DEBUGF(...)
#endif

#ifdef LOG_ESP
#define LOG(...)             \
    \
    Serial.print("Log--> "); \
    Serial.print(__VA_ARGS__);\
    Serial.print("[");\
    Serial.print(millis()-milli);\
    Serial.println("]");\
    milli=millis()
#define LOGF(sname, ...)     \
    Serial.print("Log--> "); \
    Serial.print(sname);     \
    Serial.print(": ");      \
    Serial.print(__VA_ARGS__);\
    Serial.print("[");\
    Serial.print(millis()-milli);\
    Serial.println("]");\
    milli=millis()
#else
#define LOG(...)
#define LOGF(...)
#endif

#ifdef VER_ESP
#define VER(...)             \
    Serial.print("VB--> "); \
    Serial.print(__VA_ARGS__);\
    Serial.print("[");\
    Serial.print(millis()-milli);\
    Serial.println("]");\
    milli=millis()
#define VERF(sname, ...)     \
    Serial.print("VB--> "); \
    Serial.print(sname);     \
    Serial.print(": ");      \
    Serial.print(__VA_ARGS__);\
    Serial.print("[");\
    Serial.print(millis()-milli);\
    Serial.println("]");\
    milli=millis()
#else
#define VER(...)
#define VERF(...)
#endif

#ifdef ERR_ESP
#define ERR(...) Serial.print("Err--> "); \
    Serial.print(__VA_ARGS__);\
    Serial.print("[");\
    Serial.print(millis()-milli);\
    Serial.println("]");\
    milli=millis()
#define ERRF(sname, ...)     \
    Serial.print("Err--> "); \
    Serial.print(sname);     \
    Serial.print(": ");      \
    Serial.print(__VA_ARGS__);\
    Serial.print("[");\
    Serial.print(millis()-milli);\
    Serial.println("]");\
    milli=millis()
#else
#define ERR(...)
#define ERRF(...)
#endif

#endif