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
#define ZERO 0

/////////////////EXTERN//////////////
extern uint16_t TCP_PORT;
extern Base64_AES aes;
extern void connectToServer(const char *host, int port, char *msg, size_t len);

///////////////////////////////////////////////////////////

/*********************Interface***************************/

void PerformTask(const JsonDocument &json);// this will be called when valid message is arrived/////

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
        generate();
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
    void generate()
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
    bool is_set = false;
    //////usable variable///////
    char *taskName;
    size_t taskName_len = 0;
    char *message;
    size_t message_len = 0;
    char *tag;
    size_t tag_len = 0;
    char *extra;
    size_t extra_len = 0;
    int8_t mtype = TYPE_MESSAGE;
    bool isIntent = false;
    int8_t resultcode = RESULT_UNKNOWN;
    bool is_not_consumed = false;

    ~Message()
    {
        is_set = false;
        is_not_consumed=false;
        delete id;
        delete this->tag;
        delete this->message;
        delete this->taskName;
        delete this->extra;
        this->tag_len = 0;
        this->taskName_len = 0;
        this->message_len = 0;
        this->extra_len = 0;
        this->mtype = TYPE_MESSAGE;
        this->isIntent = false;
        this->resultcode = RESULT_UNKNOWN;
    }

    void generate()
    {
        char *msg_char = new char[UNIQUE_LEN];
        getrandom(msg_char, UNIQUE_LEN);
        this->id = msg_char;
        this->is_set = true;
        this->time = micros();
        this->is_not_consumed = true;
    }

    void set(size_t ptaskName_len,
             size_t pmessage_len,
             char *ptaskName,
             char *pmessage,
             size_t ptag_len,
             char *ptag,
             size_t pextra_len,
             char *pextra,
             int8_t pmtype,
             bool isIntent,
             int8_t resultcode)
    {
        
        this->tag = new char[ptag_len + 1];
        memcpy(this->tag, ptag, ptag_len);
        this->tag[ptag_len] = 0;
        this->tag_len = ptag_len;

        this->taskName = new char[ptaskName_len + 1];
        memcpy(this->taskName, ptaskName, ptaskName_len);
        this->taskName[ptaskName_len] = 0;
        this->taskName_len = ptaskName_len;

        this->message = new char[pmessage_len + 1];
        memcpy(this->message, pmessage, pmessage_len);
        this->message[pmessage_len] = 0;
        this->message_len = pmessage_len;

        this->extra = new char[extra_len + 1];
        memcpy(this->extra, pextra, pextra_len);
        extra[pextra_len] = 0;
        this->extra_len = pextra_len;

        this->mtype = mtype;
        this->isIntent = isIntent;
        this->resultcode = resultcode;
    }
};

/////////////////////////////////////Salt Array and Pending mesage array////////////
Salt *mySalts[MAX_SALT];
uint8_t salt_counter = 0;
Message *myMsgs[MAX_MSG];
uint8_t msg_counter = 0;

////////////////////////////////////////////////////////////////////////////////
/////////////////generate salt//////////////

int generateSalt()
{
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
    mySalts[salt_counter] = new Salt(); //add the new salt to the salt array
    return salt_counter;
}

int generateMsg()
{
    msg_counter++; //add one value of salt counter
    if (msg_counter > MAX_MSG - 1)
    {
        msg_counter = 0;
    }
    Message *prev_msg = myMsgs[msg_counter];
    if (prev_msg)
    {
        if (prev_msg->is_set)
        {
            VER("Deleted previous pending msg....");
            delete prev_msg;
        }
    }
    Message *msg = new Message();
    msg->generate();
    myMsgs[msg_counter] = msg; //add the new salt to the salt array
    return msg_counter;
}

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

void constructMsgFromPendingMsg(
    const JsonDocument &json, /*We will use this json o take some data from it*/
    StaticJsonDocument<JSON_SIZE> &doc,
    Message *p_msg,
    int8_t msg_type,
    int8_t commutype = COMMUTYPE_WIFI,
    int8_t result = RESULT_UNKNOWN)
{
    ////////////lets set this sender ip/////////
    JsonObject sndr = doc.createNestedObject("sender");
    sndr["ip"] = WiFi.localIP().toString();
    sndr["port"] = TCP_PORT;
    ///////////////commutype/////////
    doc["commuType"] = commutype;
    ////////////uuidToAdd//////////
    ////////SETTING ALL NULL/////
    JsonObject uid_add = doc.createNestedObject("uuidToadd");
    uid_add["uuid"] = p_msg->id;
    doc["uuidToCheck"] = (char *)NULL;
    /////////////////////////////////
    doc["saltToAdd"] = (char *)NULL;
    doc["saltToCheck"] = json["saltToAdd"];
    /////////////////////////////////////////
    ///////result code/////////
    doc["resultCode"] = result;
    ////////message type//////
    doc["mtype"] = msg_type;
    ///////lets set intent//////
    doc["isIntent"] = p_msg->isIntent;
    ////////////////////TAG/////////////////
    doc["tag"] = p_msg->tag;
    /////////////let's set task name///////////
    doc["taskName"] = p_msg->taskName;
    //////////message/////////
    doc["message"] = p_msg->message;
    ///////////extra/////////////
    doc["extra"] = p_msg->extra;
}

void constructMsgFromMessage(
    const JsonDocument &json, /*We will use this json o take some data from it*/
    StaticJsonDocument<JSON_SIZE> &doc /*We will generate it  here*/,
    int8_t msg_type,
    int16_t salt_index,
    int16_t msg_index = -1,
    bool intent = false,
    char *taskName = (char *)NULL,
    char *message = (char *)NULL,
    char *extra = (char *)NULL,
    char *tag = (char *)NULL,
    int8_t commutype = COMMUTYPE_WIFI,
    int8_t result = RESULT_UNKNOWN)
{
    ////////////lets set this sender ip/////////
    JsonObject sndr = doc.createNestedObject("sender");
    sndr["ip"] = WiFi.localIP().toString();
    sndr["port"] = TCP_PORT;

    ///////////////commutype/////////
    doc["commuType"] = commutype;
    ////////////uuidToAdd//////////
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
    JsonObject uid_add = doc.createNestedObject("uuidToadd");
    if (msg_index < 0)
    {
        uid_add["uuid"] = (char *)NULL;
    }
    else
    {
        uid_add["uuid"] = myMsgs[salt_index]->id;
    }
    ///////////////////////////
    ///////////SaltToadd///////
    JsonObject salt_add = doc.createNestedObject("saltToAdd");
    salt_add["saltString"] = mySalts[salt_index]->salt_char;

    ////////////salt to check//////////
    if (json["saltToAdd"])
    {
        JsonObject uid_chk = doc.createNestedObject("saltToCheck");
        uid_chk["saltString"] = json["saltToAdd"]["saltString"];
    }
    else
    {
        doc["saltToCheck"] = json["saltToAdd"];
    }
    ///////result code/////////
    doc["resultCode"] = result;
    ////////message type//////
    doc["mtype"] = msg_type;
    ///////lets set intent//////
    doc["isIntent"] = intent;
    ////////////////////TAG/////////////////
    doc["tag"] = tag;
    /////////////let's set task name///////////
    doc["taskName"] = taskName;
    //////////message/////////
    doc["message"] = message;
    ///////////extra/////////////
    doc["extra"] = extra;
}

void constructMessage(
    StaticJsonDocument<JSON_SIZE> &doc /*We will generate it  here*/,
    int8_t msg_type,
    int16_t salt_index,
    int16_t msg_index = -1,
    bool intent = false,
    char *taskName = (char *)NULL,
    char *message = (char *)NULL,
    char *extra = (char *)NULL,
    char *tag = (char *)NULL,
    int8_t commutype = COMMUTYPE_WIFI,
    int8_t result = RESULT_UNKNOWN)
{
    ////////////lets set this sender ip/////////
    JsonObject sndr = doc.createNestedObject("sender");
    sndr["ip"] = WiFi.localIP().toString();
    sndr["port"] = TCP_PORT;

    ///////////////commutype/////////
    doc["commuType"] = commutype;
    ////////////uuidToAdd//////////
    ////Sorry it is hard to generate a uuid in microcontroller so i am leaving it by taking some random char array///
    JsonObject uid_add = doc.createNestedObject("uuidToadd");
    if (msg_index < 0)
    {
        uid_add["uuid"] = (char *)NULL;
    }
    else
    {
        uid_add["uuid"] = myMsgs[salt_index]->id;
    }
    doc["uuidToCheck"] = (char *)NULL;
    ///////////////////////////
    ///////////SaltToadd///////
    JsonObject salt_add = doc.createNestedObject("saltToAdd");
    salt_add["saltString"] = mySalts[salt_index]->salt_char;
    doc["saltToCheck"] = (char *)NULL;
    ///////result code/////////
    doc["resultCode"] = result;
    ////////message type//////
    doc["mtype"] = msg_type;
    ///////lets set intent//////
    doc["isIntent"] = intent;
    ////////////////////TAG/////////////////
    doc["tag"] = tag;
    /////////////let's set task name///////////
    doc["taskName"] = taskName;
    //////////message/////////
    doc["message"] = message;
    ///////////extra/////////////
    doc["extra"] = extra;
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

    size_t crypted_len = strnlen(crypted, expected_msg_len); //lets take know the length till "0" for some case
    ///Lets make the line ended by "\r\n" this will make server to disconnect
    crypted[crypted_len] = 13;     //"\r"
    crypted[crypted_len + 1] = 10; //"\n"
    crypted[crypted_len + 2] = 0;  //"0"
    crypted_len = crypted_len + 3;
    connectToServer(ip, port, crypted, crypted_len);
    //Dont panic we will delete the "crypted" in another functio... don't delete it here
}

///////////////////////////////////////////////////////////////
void validate(char *data, size_t len)
{

    BYTE_LOG(data, len);
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
    const int8_t msgType = doc["mtype"];
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
        size_t index = generateSalt();
        ///////Okey we have add an new salt to the salt array
        //// Now,let's custruct a new message to send
        ////////////////////////////////
        StaticJsonDocument<JSON_SIZE> newdoc;
        constructMsgFromMessage(doc, newdoc, TYPE_INIT, index);
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
            if (compare_ID(q_salt, p_salt->salt_char, p_salt->time, curmicro))
            {
                validsalt = true;
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
        // lets get the message uuid to check
        LOG("Init... please send actual meaage");
        const char *init_uuid = doc["uuidToCheck"]["uuid"];
        LOGF("UUID to check", init_uuid);

        /// loop inside the array to check if there is the message
        unsigned long curmicro_msg = micros();
        boolean validMsg = false;
        //// if it is not there do nothing
        int m = 0;
        Message *p_msg;
        ///////Checking there is valid message in message array///////
        for (;;)
        {
            if (m >= MAX_MSG)
            {
                break;
            }
            ///////////////////////
            p_msg = myMsgs[m];
            if (p_msg) //if the salt is not null
            {
                if (compare_ID(init_uuid, p_msg->id, p_msg->time, curmicro_msg))
                {
                    validMsg = true;
                    break;
                }
            }
            //////////////////////////
            m++;
        }
        if (!validMsg)
        {
            ERR("Unfortunately there is no such message");
            return;
        }
        if (!p_msg->is_not_consumed)
        {
            ERR("Valid message but already consumed");
            return;
        }
        p_msg->is_not_consumed = false;
        LOGF("Pennding msg", p_msg->message);
        // if YES.... change the mesage gtipe to TYPE_MESSAGE and send back again
        StaticJsonDocument<JSON_SIZE> newpendingJson;
        ////////////////////////////////
        constructMsgFromPendingMsg(doc, newpendingJson, p_msg, TYPE_MESSAGE);
        int p_sz = measureJson(newpendingJson) + 2;
        char *pen_msg2send = new char[p_sz];
        serializeJson(newpendingJson, pen_msg2send, p_sz);
        pen_msg2send[p_sz - 1] = 0; //terminating the string with zero in case....
        DEBUGF("Pending message to send", pen_msg2send);
        handleEncryptedsend(pen_msg2send, p_sz, SenderIp, SenderPort);
        delete pen_msg2send;
        return; //return as w have send a valid salt
    }
    if (msgType == TYPE_MESSAGE)
    {
        // doc["tag"] doc["taskName"] doc["message"] doc["extra"]
        LOG("We have to do something");
        SRLF("ESP_RCV:", (const char *)doc["tag"]);
        PerformTask(doc);
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