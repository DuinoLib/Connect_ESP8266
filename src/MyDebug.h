
#ifndef MYDEBUG_H
#define MYDEBUG_H
///DEFFINE//////
#define DEBUG_ESP
#define LOG_ESP
#define VER_ESP
#define ERR_ESP
#define LOG_BYTE
///////////////

unsigned long milli = 0;

#ifdef DEBUG_ESP
#define DEBUG(...)                  \
    Serial.print("Debug--> ");      \
    Serial.print(__VA_ARGS__);      \
    Serial.print("[");              \
    Serial.print(millis() - milli); \
    Serial.println("]");            \
    milli = millis()

#define DEBUGF(sname, ...)          \
    Serial.print("Debug--> ");      \
    Serial.print(sname);            \
    Serial.print(": ");             \
    Serial.print(__VA_ARGS__);      \
    Serial.print("[");              \
    Serial.print(millis() - milli); \
    Serial.println("]");            \
    milli = millis()

#else
#define DEBUG(...)
#define DEBUGF(...)
#endif

#ifdef LOG_ESP
#define LOG(...)                    \
    Serial.print("Log--> ");        \
    Serial.print(__VA_ARGS__);      \
    Serial.print("[");              \
    Serial.print(millis() - milli); \
    Serial.println("]");            \
    milli = millis()
#define LOGF(sname, ...)            \
    Serial.print("Log--> ");        \
    Serial.print(sname);            \
    Serial.print(": ");             \
    Serial.print(__VA_ARGS__);      \
    Serial.print("[");              \
    Serial.print(millis() - milli); \
    Serial.println("]");            \
    milli = millis()

#else
#define LOG(...)
#define LOGF(...)
#endif

#ifdef LOG_BYTE
#define BYTE_LOG(...)        \
    Serial.print("Log--> "); \
    byteLog(__VA_ARGS__)

#define BYTE_LOGF(sname, ...) \
    Serial.print("Log--> ");  \
    Serial.print(sname);      \
    Serial.print(": ");       \
    byteLog(__VA_ARGS__)
void byteLog(char *ch, size_t len)
{
    Serial.print("[");
    for (size_t i = 0; i < len; i++)
    {
        Serial.print((uint8_t)ch[i]);
        if (i + 1 == len)
        {
            break;
        }
        Serial.print(",");
    }
    Serial.println("]");
}
void byteLog(const char *ch, size_t len)
{
    Serial.print("[");
    for (size_t i = 0; i < len; i++)
    {
        Serial.print((uint8_t)ch[i]);
        if (i + 1 == len)
        {
            break;
        }
        Serial.print(",");
    }
    Serial.println("]");
}
#else
#define BYTE_LOG(...)
#endif

#ifdef VER_ESP
#define VER(...)                    \
    Serial.print("VB--> ");         \
    Serial.print(__VA_ARGS__);      \
    Serial.print("[");              \
    Serial.print(millis() - milli); \
    Serial.println("]");            \
    milli = millis()
#define VERF(sname, ...)            \
    Serial.print("VB--> ");         \
    Serial.print(sname);            \
    Serial.print(": ");             \
    Serial.print(__VA_ARGS__);      \
    Serial.print("[");              \
    Serial.print(millis() - milli); \
    Serial.println("]");            \
    milli = millis()
#else
#define VER(...)
#define VERF(...)
#endif

#ifdef ERR_ESP
#define ERR(...)                    \
    Serial.print("Err--> ");        \
    Serial.print(__VA_ARGS__);      \
    Serial.print("[");              \
    Serial.print(millis() - milli); \
    Serial.println("]");            \
    milli = millis()
#define ERRF(sname, ...)            \
    Serial.print("Err--> ");        \
    Serial.print(sname);            \
    Serial.print(": ");             \
    Serial.print(__VA_ARGS__);      \
    Serial.print("[");              \
    Serial.print(millis() - milli); \
    Serial.println("]");            \
    milli = millis()
#else
#define ERR(...)
#define ERRF(...)
#endif

#endif