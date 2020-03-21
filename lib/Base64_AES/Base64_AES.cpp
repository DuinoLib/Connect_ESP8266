
#include "Base64_AES.h"

Base64_AES::Base64_AES() {}

void Base64_AES::setkey(byte *mykey) {
  key = mykey;
}

int Base64_AES::expected_encrypted_b64_len(int msg_length) {
  return base64_encode_expected_len ((msg_length + N_BLOCK - msg_length % N_BLOCK) + N_BLOCK);// this additional "N_BLOCK" is for adding iv key.
}

int Base64_AES::expected_decrypted_b64_len(int msg_length) {
  return base64_decode_expected_len(msg_length) - N_BLOCK;
}

void Base64_AES::encrypt_b64(char * msg, int msg_length, char * encrypted_b64_message) {
  /**
    convert the message to byte array
  */
  byte *msg_bytes = new byte[msg_length];
  for (int i = 0; i < msg_length; i++) {
    msg_bytes[i] = (byte) msg[i];
  }
  /**
     Variables
  */
  int padedLength = msg_length + N_BLOCK - msg_length % N_BLOCK;
  byte iv [N_BLOCK] ;
  byte real_iv [N_BLOCK] ;
  byte *cipher = new byte[padedLength];
  /**
     Encrypt the messgae
  */
  aes.iv_inc();
  for (int i = 0; i < N_BLOCK; i++) {
    iv[i] = random(0, 255);// lets generate random iv like this
  }
  // clone the iv
  memcpy(real_iv, iv, N_BLOCK);//clone the iv key as it get consume during encryption
  //lets clone the key also because "i want to do so"
  byte temp_key [N_BLOCK] ;
  memcpy(temp_key, key, N_BLOCK);
  //encrypt the messgae
  aes.do_aes_encrypt(msg_bytes, msg_length, cipher, temp_key, 128, iv);//we use 128 bit aes encryption
  free(msg_bytes);
  // attatch the iv at the front with the encrypted message
  int final_cipher_len = padedLength + N_BLOCK;
  byte *final_cipher = new byte[final_cipher_len];
  for (int i = 0; i < N_BLOCK; i++) {
    final_cipher[i] = real_iv[i];
  }
  for (int i = 0; i < padedLength; i++) {
    final_cipher[N_BLOCK + i] = cipher[i];
  }
  //  Serial.println("\nMessage   :");
  //  aes.printArray(msg_bytes, msg_length);
  //  Serial.println("\nKey   :");
  //  aes.printArray(secret_key, 16);
  //  Serial.println("\nTemp Key   :");
  //  aes.printArray(temp_key, 16);
  //  Serial.println("\nCypher   :");
  //  aes.printArray(cipher, padedLength);
  //  Serial.println("\nReal IV   :");
  //  aes.printArray(real_iv, padedLength);
  //  Serial.println("\IV + Cypher   :");
  //  aes.printArray(final_cipher, final_cipher_len);
  /**
     decode the messgae
     the encrypted message is "final_cipher" which contain the iv... lets encode this string
  */
  //convert the byte array to char array
  const char *cipher_char = reinterpret_cast<const char*>(final_cipher);
  // now lets do base64 encode
  base64_encode_chars(cipher_char, final_cipher_len, encrypted_b64_message);
  free(cipher);
  free(final_cipher);
}


void Base64_AES::decrypt_b64(char * encrypted, int msg_length, char * decrypted_message) {
  /**
     convert the base64 encoded message to byte array
  */
  int expected_decode_length = base64_decode_expected_len(msg_length);
  char *decoded = new char[expected_decode_length];
  base64_decode_chars(encrypted, msg_length, decoded);
  /**
     decrypt the message.
     we will get decrypted byte array
  */
  ///////variables////////

  byte iv[N_BLOCK] ;
  int encrypted_bytes_len = expected_decode_length - N_BLOCK;
  byte *ecrypted_bytes = new byte [encrypted_bytes_len] ;
  byte *decrypted_bytes = new byte [encrypted_bytes_len] ;

  // seperate the iv key and message from the decoded message
  for (int i = 0; i < N_BLOCK; i++) {
    iv[i] = decoded[i];
  }
  for (int i = 0; i < encrypted_bytes_len; i++) {
    ecrypted_bytes[i] = decoded[N_BLOCK + i];
  }
  //decrypt the message
  //lets clone the key also because "i want to do so"
  byte temp_key [N_BLOCK] ;
  memcpy(temp_key, key, N_BLOCK);
  aes.do_aes_decrypt(ecrypted_bytes, encrypted_bytes_len, decrypted_bytes, temp_key, 128, iv); //we use 128 bit aes encryption
  free(decoded);
  free(ecrypted_bytes);

  /**
     covert the byte array back to char array and store it to "decrypted_message"
  */
  for (int i = 0; i < encrypted_bytes_len; i++) {
    decrypted_message[i] = (char) decrypted_bytes[i];
    //    Serial.println( decrypted_bytes[i]);
  }
  free(decrypted_bytes);
}
