/**
   Library used: https://github.com/spaniakos/AES

*/

#ifndef Base64Aes
#define Base64Aes

#include <AES.h>
#include <libb64/cencode.h>
#include <libb64/cdecode.h>

////////////////////AES FUNCTGIONS///////////////////

class Base64_AES {
  public:
    Base64_AES(uint16_t bit);
    boolean decrypt_b64(char * encrypted, int msg_length, char * decrypted_message);
    void encrypt_b64(char * msg, int msg_length, char * encrypted_b64_message);
    int expected_encrypted_b64_len(int msg_length);
    int expected_decrypted_b64_len(int msg_length);
    void setkey(byte *mykey);
  private:
    AES aes;
    byte *key;
    uint16_t _bit;
};
#endif
