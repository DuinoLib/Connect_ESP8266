#include "Base64_AES.h"

Base64_AES aes;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println();
  byte *secret_key = (unsigned char*)"0123456789010123";// it should be 16 letter
  aes.setkey(secret_key);
  char hello[] = "Hello World";
  int epected_encode_legth = aes.expected_encrypted_b64_len(sizeof(hello));
  char *baseencoded = new char[epected_encode_legth];
  aes.encrypt_b64(hello, sizeof(hello), baseencoded);
  Serial.println("Encrypted: ");
  Serial.println(baseencoded);

  ////////Decryption////////////
  char encodeddata[] = "V6Krbb8PNJ6ksebEq+ozEBgDj0fQ6Uu0OlxUL+hoNCrCETug+dW2tyMKppv5reRS";
  int epected_msg_len = aes.expected_decrypted_b64_len(sizeof(encodeddata));
  char *decryptedmsg = new char[epected_msg_len];
  aes.decrypt_b64(encodeddata, sizeof(encodeddata), decryptedmsg);
  Serial.println("Dencrypted: ");
  Serial.println(decryptedmsg);

}

void loop() {
  // put your main code here, to run repeatedly:

}
