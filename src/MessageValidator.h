#ifndef MESSAGE_VALIDATOR
#define MESSAGE_VALIDATOR

#include <ArduinoJson.h>
#include <Base64_AES.h>
#include <MyDebug.h>
#include <MessageValidatorUtils.h>

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

////////////////////////////////DEFINE USER CHOICE////////////////////
#define MAX_SALT 3
#define MAX_MSG 3
#define JSON_SIZE 2000

/////////////////EXTERN//////////////
extern uint16_t TCP_PORT;
extern Base64_AES aes;
extern void connectToServer(const char *host, int port, char *msg, size_t len);

///////////////////////////////////////////////////////////

/**
 * 
 * ***********************CLASS**********************
 * 
 * 
 */

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

/////////////////////////////////////Salt Array and Pending mesage array////////////
Salt *mySalts[MAX_SALT];
uint8_t salt_counter = 0;
Message *myMsgs[MAX_MSG];

////////////////////////////////////////////////////////////////////////////////
/**
 *
 **************************DEFINING FUCTIONS***********************
 * 
 */

/**
 * 
 ************** This function will generate a json doc whic is a mesage *********************
 * 
 */

void constructMsg(
    const JsonDocument &json, /*We will use this json o take some data from it*/
    StaticJsonDocument<JSON_SIZE> &doc /*We will generate it  here*/,
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
    ////Sorry it is hard to generate a uuid in microcontroller so i am leaving it by taking some random char array///
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

///////////////////////////Encrypted Mesasge Sender//////////////

void handleEncryptedsend(char *msg, uint16_t len, const char *ip, uint16_t port)
{
    /**
     *What we are doing here is -->  taking the message and encrypt 
     *it and pass to a function to send it to respective TCP server 
     */
    ////////Lets encrypt the message
    size_t expected_msg_len = aes.expected_encrypted_b64_len(len);
    if (expected_msg_len > 2000)
    {
        expected_msg_len = 2000; // limit the message length by 2000
    }
    char *crypted = new char[expected_msg_len + 3]; // we add 3 extra byte to store --> "\r\n\0"
    aes.encrypt_b64(msg, len, crypted);
    ///////Okeyy we have encrypted the message
    size_t crypted_len = strlen(crypted); //lets take know the lengt till "0" for some case

    if (expected_msg_len - crypted_len < 3)
    {
        crypted_len = expected_msg_len;
        ///Lets make the line ended by "\r\n" this will make server to disconnect
        crypted[crypted_len - 2] = 13; //"\r"
        crypted[crypted_len - 1] = 10; //"\n"
        crypted[crypted_len] = 0;      //"0"
    }
    else
    {
        ///Lets make the line ended by "\r\n" this will make server to disconnect
        crypted[crypted_len] = 13;     //"\r"
        crypted[crypted_len + 1] = 10; //"\n"
        crypted[crypted_len + 2] = 0;  //"0"
        crypted_len = crypted_len + 3;
    }
    connectToServer(ip, port, crypted, crypted_len);
    //Dont panic we will delete the "crypted" in another functio... don't delete it here
}

///////////////////////////////////////////////////////////////
void validate(char *data, size_t len)
{
    /**
     * In this fuction we take a message from the sender and w check if it is valid
     * 
     * if it is valid do action for each type of the action
     * 
    */
    StaticJsonDocument<JSON_SIZE> doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error)
    {
        ERRF("deserializeJson() failed", error.c_str());
        return; //return as the message is not able to parse as json
    }
    ////cheeck message is init type/salt to check is null or not////
    const uint8_t msgType = doc["mtype"];
    /////check sender is valid////
    if (!doc["sender"])
    {
        ERR("Sender is NULL");
        return; //return as sender is null
    }
    /////Store the sender data
    const char *SenderIp = doc["sender"]["ip"];
    uint16_t SenderPort = doc["sender"]["port"];
    ///////////////////////////
    if (!SenderIp || !SenderPort)
    {
        ERR("Sender IP Port is NULL");
        return; //return again as ip address and port is null
    }
    /**
     *Now let us check if there is "saltToCheck" if not lets send back with a valid salt
     * 
     */
    if (!doc["saltToCheck"])
    {
        VER("saltToCheck is null");
        ///////////////Here we are adding a new Salt to the Salt Array//////////////////
        salt_counter++; //add one value of salt counter
        if (salt_counter > MAX_SALT - 1)
        {
            salt_counter = 0;
        }
        Salt *prev_salt = mySalts[salt_counter];
        if (prev_salt)
        {
            VER("Deleted previous salt....");
            delete prev_salt;
        }
        Salt *mysalt = new Salt();
        mySalts[salt_counter] = mysalt; //add the new salt to the salt array
        ///////Okey we have add an new salt to the salt aray
        //// Now,let's custruct a new message to send
        ////////////////////////////////
        StaticJsonDocument<JSON_SIZE> newdoc;
        constructMsg(doc, newdoc, TYPE_INIT, salt_counter);
        int sz = measureJson(newdoc) + 2;
        char *msg2send = new char[sz];
        serializeJson(newdoc, msg2send, sz);
        msg2send[sz - 1] = 0; //terminating the string with zero in case....
        DEBUGF("Messages to send", msg2send);
        /// lets send the message/////
        handleEncryptedsend(msg2send, sz, SenderIp, SenderPort);
        delete msg2send;
        return; //return as w have send a valid salt
    }
    VER("There is salt to check if it is valid");
    /// check salt is valid//// first delete the expired salt then check
    const char *q_salt = doc["saltToCheck"]["saltString"];
    VERF("Q_salt", q_salt);
    if (!q_salt)
    {
        ERR("Salt String is Emptied");
        return; //return as it is not a valid salt
    }
    ////////check two string is equal//////
    boolean validsalt = false;
    unsigned long curmicro = micros(); // this micros seconds is to check the if the salt is expiered or not
                                       ///lets interate over the salt array......
    ////////////looping//////////////
    int k = 0;
    for (;;)
    {
        if (k >= MAX_SALT)
        {
            break;
        }
        ///////////////////////
        Salt *p_salt = mySalts[k];
        if (p_salt) //if the salt is not null
        {
            if (compareSalt(q_salt,p_salt->salt_char, p_salt->time,curmicro))
            {
                validsalt=true;
                break;
            }
        }
        //////////////////////////
        k++;
    }
    //////Now we know  if the salt is valid or not
    if (!validsalt)
    {
        ERR("Invalid Salt!!!!");
        return;
    }
    VER("Valid Valid Valid");
    ////////////////////OKEY The salt is valid//////////////

    // check the message type is init.....
    //if the message is INIT type//// send the aactual meaage
    if (msgType == TYPE_INIT)
    {
        LOG("Init... please send actual meaage");
        return;
    }
    if (msgType == TYPE_MESSAGE)
    {
        LOG("We have to do something");
        return;
    }

    if (msgType == TYPE_RESPOSNE)
    {
        LOG("Response of previous is recieve");
        return;
    }
    ERR("Must not reach here.....");
}

#endif