
#include <ArduinoJson.h>
#include <Base64_AES.h>

#define NOTIFICATION_CHANNEL_ID "com.unbi.connect"
#define BOOTCOMPLETE "android.intent.action.BOOT_COMPLETED"
#define MAXIMUM_TIME 10 * 60 * 1000
#define MIN_POP_TIME 10 * 1000
#define UUID_PREFIX "uuid_"
#define SALT_PREFIX "salt_"
#define TYPE_INIT 0
#define TYPE_RESPOSNE 1
#define TYPE_MESSAGE 2
#define RESULT_UNKNOWN 0
#define RESULT_SUCCESS 1
#define RESULT_FAILURE -1
#define LOCAL_IP "0.0.0.0"
#define TO_REDIRECT 1
#define TO_TRIGGER 2
#define TO_NOT_TODO -1
#define MSG_INVALID 0
#define MSG_VALID 1
#define COMMUTYPE_WIFI 1
#define COMMUTYPE_BLUETOOTH 2

#define DATALIST_SALT "salttype"
#define DATALIST_MSG "pendingMsg"

////////////////////////////////DEFINE////////////////////
#define UNIQUE_LEN 16 // length of the salt or msessage id
#define MAX_SALT 3
#define MAX_MSG 3

#define MAX_PENDING_TIME 10000000 //10 seconds in microseconds

/////////////////EXTERN//////////////
extern uint16_t TCP_PORT;
extern Base64_AES aes;
extern void connectToServer(const char *host, int port, char *msg, size_t len);

///////////////////////////////////////////////////
///////////////////////////////////////////////////////////
void getrandom(char *ran, size_t len)
{
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

//////////////***********************CLASS********************////////////////////

class Salt
{
public:
    Salt()
    {
        generateSalt();
    }
    unsigned long time = 0;
    char *salt_char;
    boolean generated = false;
    ~Salt()
    {
        delete this->salt_char;
        this->time = 0;
        this->generated = false;
    }

private:
    void generateSalt()
    {
        char *salt_char = new char[UNIQUE_LEN];
        getrandom(salt_char, UNIQUE_LEN);
        this->salt_char = salt_char;
        this->generated = true;
        this->time = micros();
    }
};

class Message
{
public:
    unsigned long time = 0;
    char *id;
    char *msg;
    bool is_set = false;
};

/////////////////////////////////////Salt array And message array////////////

Salt *mySalts[MAX_SALT];
uint8_t salt_counter = 0;

Message *myMsgs[MAX_MSG];
//////////////////Pop the salt/////////////////

//////////////////////////////////////Message Constructor////////////

void constructMsg(
    const JsonDocument &json,
    StaticJsonDocument<1500> &doc,
    uint8_t msg_type,
    uint16_t salt_index,
    uint16_t msg_index = -1,
    bool intent = false,
    char *taskName = NULL,
    char *message = NULL,
    char *extra = NULL,
    char *tag = NULL,
    uint8_t commutype = COMMUTYPE_WIFI,
    int8_t result = RESULT_UNKNOWN)
{

    ///set uuidtocheck
    if (json["uuidToadd"])
    {
        JsonObject uid_chk = doc.createNestedObject("uuidToCheck");
        uid_chk["uuid"] = json["uuidToadd"]["uuid"]; // uuidtocheck must be uuid to add
    }
    else
    {
        doc["uuidToCheck"] = json["uuidToadd"];
    }
    ////////////////////////////
    ///////lets set intent//////
    doc["isIntent"] = intent;
    /////////////let's set task name///////////
    doc["taskName"] = taskName;
    ////////////lets set this sender ip/////////
    JsonObject sndr = doc.createNestedObject("sender");
    sndr["ip"] = WiFi.localIP().toString();
    sndr["port"] = TCP_PORT;
    ////////////uuidToAdd//////////

    JsonObject uid_add = doc.createNestedObject("uuidToadd");
    if (msg_index < 0)
    {
        uid_add["uuid"] = NULL;
    }
    else
    {
        uid_add["uuid"] = mySalts[salt_index]->salt_char;
    }
    ///////////extra/////////////
    doc["extra"] = extra;
    ////////message type//////
    doc["mtype"] = msg_type;
    ///////////////////////////
    ///////////SaltToadd///////
    JsonObject salt_add = doc.createNestedObject("saltToAdd");
    salt_add["saltString"] = mySalts[salt_index]->salt_char;
    // salt_add["milli"] = mySalts[salt_index]->time;
    ////////////////////TAG/////////////////
    doc["tag"] = tag;
    ///////////////commutype/////////
    doc["commuType"] = commutype;
    ///////result code/////////
    doc["resultCode"] = result;
    ////////////salt to check//////////
    if (json["saltToAdd"])
    {
        JsonObject uid_chk = doc.createNestedObject("saltToCheck");
        uid_chk["saltString"] = json["saltToAdd"]["saltString"];
        // uid_chk["milli"] = (unsigned long)json["saltToAdd"]["milli"];
    }
    else
    {
        json["saltToCheck"] = json["saltToAdd"];
    }
    //////////message/////////
    doc["message"] = message;
}

///////////////////////////ENCRYpted Mesasge Sender//////////////

void handleEncryptedsend(char *msg, uint16_t len, const char *ip, uint16_t port)
{
    // Serial.println("\nSending msessage...\n");
    // Serial.println(ip);
    // Serial.println(port);
    // Serial.println(msg);

    ////////Do something here//////////

    Serial.println(len);
    size_t expected_msg_len = aes.expected_encrypted_b64_len(len);
    if (expected_msg_len > 1500)
    {
        expected_msg_len = 1500;
    }
    char *crypted = new char[expected_msg_len + 2];
    aes.encrypt_b64(msg, len, crypted);
    //////////DO SOMETHING HERE/////
    size_t crypted_len = strlen(crypted);
    ///Lets make the line ended by "\r\n" this will make server to disconnect
    crypted[crypted_len] = 13;     //"\r"
    crypted[crypted_len + 1] = 10; //"\n"
    crypted[crypted_len + 2] = 0;  //"0"
    crypted_len = crypted_len + 3;
    // Serial.println("\nEnrypt len");
    // Serial.println(crypted);
    connectToServer(ip, port, crypted, crypted_len);
}

///////////////////////////////////////////////////////////////
void validate(char *data, size_t len)
{
    Serial.println("\nValidate");
    // Serial.println(data);
    StaticJsonDocument<1500> doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error)
    {
        Serial.println();
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }
    const char *sensor = doc["taskName"];
    Serial.println("Taask name:");
    Serial.print(sensor);

    ////cheeck message is init type/salt to check is null or not////
    const uint8_t msgType = doc["mtype"];

    /////check sender is valid////
    if (!doc["sender"])
    {
        Serial.println("\nSender is NULL");
        return;
    }

    const char *SenderIp = doc["sender"]["ip"];
    uint16_t SenderPort = doc["sender"]["port"];
    if (!SenderIp || !SenderPort)
    {
        Serial.println("\nSender IP Port is NULL");
        return;
    }

    /// if salt to check is null send some valid salt///
    // if(true)
    if (!doc["saltToCheck"])
    {

        Serial.println("\n Init or Salt to check is null");
        ///////////////Here we are adding a new Salt to the Salt Array//////////////////

        salt_counter++; //add one value of salt counter
        if (salt_counter > MAX_SALT - 1)
        {
            salt_counter = 0;
        }
        Salt *prev_salt = mySalts[salt_counter];
        if (prev_salt)
        {
            ///////////////////////////
            Serial.println(prev_salt->salt_char);
            Serial.println(prev_salt->time);
            if (prev_salt->generated)
            {
                Serial.println("Generated");
            }
            else
            {
                Serial.println("Not Generated");
            }
            ///////////////////////////
            delete prev_salt;
        }
        else
        {
            Serial.println("Prev Salt was empty");
        }
        Salt *mysalt = new Salt();
        mySalts[salt_counter] = mysalt;

        ///////////////FINISH--->>>>Here we are adding a new Salt to the Salt Array//////////////////
        // Lets create the new message
        ///////////////////////////////
        StaticJsonDocument<1500> newdoc;
        constructMsg(doc, newdoc, TYPE_INIT, salt_counter);
        int sz = measureJson(newdoc) + 2;
        char *msg2send = new char[sz];
        serializeJson(newdoc, msg2send, sz);
        msg2send[sz - 1] = 0;
        // Serial.println(msg2send);
        /////////serialize///////
        /// lets send the message/////

        handleEncryptedsend(msg2send, sz, SenderIp, SenderPort);

        delete msg2send;
    }

    /// if it is not ////
    Serial.println("\nThere is salt to check");
    /// check salt is valid//// first delete the expired salt then check
    const char *q_salt = doc["saltToCheck"]["saltString"];
    Serial.print("\nQ_salt: ");
    Serial.print(q_salt);
    if (!q_salt)
    {
        Serial.println("Salt String is Emptied");
        return;
    }
    ////////check two string is equal//////
    boolean validsalt = false;
    unsigned long curmicro = micros();
    for (size_t i = 0; i < MAX_SALT; i++)
    {
        Salt *p_salt = mySalts[i];
        if (p_salt)
        {
            if (curmicro - p_salt->time < MAX_PENDING_TIME)
            {
                bool eq = true;
                for (size_t i = 0; i < UNIQUE_LEN; i++)
                {
                    if (q_salt[i] != p_salt->salt_char[i])
                    {
                        eq = false;
                        break;
                    }
                }
                if (eq)
                {
                    validsalt = true;
                    // delete p_salt;
                    Serial.print("\nEqual: ");
                    Serial.print(q_salt);
                    Serial.print(",");
                    Serial.print(p_salt->salt_char);
                    break;
                }
                else
                {
                    Serial.print("\nNot Equal: ");
                    Serial.print(q_salt);
                    Serial.print(",");
                    Serial.print(p_salt->salt_char);
                }
            }
            else
            {
                Serial.println("\nExpired salt ");
                Serial.println(p_salt->salt_char);
                Serial.println(curmicro - p_salt->time);
            }
        }
        if (validsalt)
        {
            break;
        }
    }
    if (!validsalt)
    {
        Serial.println("\nInvalid Salt!!!!");
        return;
    }

    Serial.println("\nValid Valid Valid");
    ////////////////////OKEY The salt is valid//////////////

    // check the message type is init.....
    //if the message is INIT type//// send the aactual meaage
    if (msgType == TYPE_INIT)
    {
        Serial.println("\nInit... please send actual meaage");

        return;
    }
    if (msgType == TYPE_MESSAGE)
    {
        Serial.println("\n We have to do something");

        return;
    }

    if (msgType == TYPE_RESPOSNE)
    {
        Serial.println("\nResponse of previous is recieve");

        return;
    }

    Serial.println("\nMust not Recieve here.....");
}
