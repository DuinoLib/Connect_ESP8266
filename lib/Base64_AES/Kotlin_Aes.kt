package duinoCypher

import java.security.SecureRandom
import java.util.*
import javax.crypto.Cipher
import javax.crypto.spec.IvParameterSpec
import javax.crypto.spec.SecretKeySpec

val key = "12345678123456781234567812345678".toByteArray()

fun main() {
    val encrypteddata = "03cPL8GOWP2/aISD5S7dRJrVYOHK3bFWAo4vV7nt51Y="
    println(decryptFromBaseEncoded(encrypteddata))
    val enccrypted = encryptToBaseEncoded("Have a nice day 👍😉!!!")
    println(enccrypted)
    println(decryptFromBaseEncoded(enccrypted))
}

fun decryptFromBaseEncoded(encrypted_msg: String): String {
    val data = Base64.getDecoder().decode(encrypted_msg)
    val ivbyte = Arrays.copyOf(data, 16);
    val msgbyte = Arrays.copyOfRange(data, 16, data.size)
    val iv = IvParameterSpec(ivbyte)
    val skeySpec = SecretKeySpec(key, "AES")
    val dcipher = Cipher.getInstance("AES/CBC/NoPadding")
    dcipher.init(Cipher.DECRYPT_MODE, skeySpec, iv)
    val decryptedbytes = dcipher.doFinal(msgbyte)
    //lets remove the padding
    val padded_byte1: Byte = 10// Arduino padded with "10"
    val padded_byte2: Byte = 3// Arduino padded with "10"
    for (num in 0 until dcipher.blockSize) {
        val index = decryptedbytes.size - 1 - num
        if (decryptedbytes[index] == padded_byte1||decryptedbytes[index] == padded_byte2) {
            decryptedbytes[index] = 0
        } else {
            break
        }
    }
    ///////////////////////////
    return String(decryptedbytes)
}

fun encryptToBaseEncoded(msg: String): String {
    val cipher = Cipher.getInstance("AES/CBC/NoPadding")
    val randomSecureRandom = SecureRandom()
    val ivbytes = ByteArray(cipher.blockSize)
    randomSecureRandom.nextBytes(ivbytes)
    val ivParams = IvParameterSpec(ivbytes)
    val skeySpec = SecretKeySpec(key, "AES")
    cipher.init(Cipher.ENCRYPT_MODE, skeySpec, ivParams)
    //lets do padding
    val msg_byte = msg.toByteArray();
    val block_size = cipher.blockSize
    val paddable = block_size - msg_byte.size % block_size
    val padded_msg_byte = ByteArray(msg_byte.size + paddable)
    System.arraycopy(msg_byte, 0, padded_msg_byte, 0, msg_byte.size)
    val byte: Byte = 0// Zerro padding is good...
    for (num in 0 until paddable) {
        padded_msg_byte[msg_byte.size + num] = byte
    }
    //////////////////////////////////////////////////////////
    val ciphered = cipher.doFinal(padded_msg_byte)
    val cryptedmsg = ByteArray(ivbytes.size + ciphered.size)
    System.arraycopy(ivbytes, 0, cryptedmsg, 0, ivbytes.size)
    System.arraycopy(ciphered, 0, cryptedmsg, ivbytes.size, ciphered.size)
    return Base64.getEncoder().encodeToString(cryptedmsg)
}
fun hexStringToByteArray(s: String): ByteArray {
    val len = s.length
    val data = ByteArray(len / 2)
    var i = 0
    while (i < len) {
        data[i / 2] = ((Character.digit(s[i], 16) shl 4) + Character.digit(s[i + 1], 16)).toByte()
        i += 2
    }
    return data
}

fun ByteArray.toHexString() = asUByteArray().joinToString("") { it.toString(16).padStart(2, '0') }
