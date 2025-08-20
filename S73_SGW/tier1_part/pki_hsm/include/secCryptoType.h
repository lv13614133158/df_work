#ifndef _SECURITY_CRYPTO_TYPES_H_
#define _SECURITY_CRYPTO_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <err.h>
#include <string.h>
#include <stdbool.h>


typedef enum {
    SecCrypto_Alg_Hash_MD5,
    SecCrypto_Alg_Hash_SHA1,
    SecCrypto_Alg_Hash_SHA224,
    SecCrypto_Alg_Hash_SHA256,
    SecCrypto_Alg_Hash_SHA384,
    SecCrypto_Alg_Hash_SHA512,
    SecCrypto_Alg_Hash_SM3,
    SecCrypto_Alg_Hash_MAX,
} SecCrypto_Alg_Hash;

typedef enum {
    SecCrypto_Alg_Key_SYM_KEY_128 = 0,
    SecCrypto_Alg_Key_SYM_KEY_192,
    SecCrypto_Alg_Key_SYM_KEY_256,
    SecCrypto_Alg_Key_SYM_KEY,
    SecCrypto_Alg_Key_ASYM_KEY_1024 = 4, //128 * 3(n, e, d)
    SecCrypto_Alg_Key_ASYM_KEY_2048, //256 * 3(n, e, d)
    SecCrypto_Alg_Key_ASYM_KEY_3072, //384 * 3(n, e, d)
    SecCrypto_Alg_Key_ASYM_KEY_4096, //512 * 3(n, e, d)
    SecCrypto_Alg_Key_ECC_KEY_P192, //24 + 48 = 72
    SecCrypto_Alg_Key_ECC_KEY_P256, //32 + 64 = 96
    SecCrypto_Alg_Key_ECC_KEY_P384, //48 + 96 = 144
    SecCrypto_Alg_Key_ECC_KEY_P521, //66 + 132 = 198
    SecCrypto_Alg_Key_ECC_KEY_E521, //66 + 132 = 198
    SecCrypto_Alg_Key_ECC_KEY_SM2, //32 + 64 = 96
    SecCrypto_Alg_Key_ALL_KEY_ALG_MAX,
} SecCrypto_Alg_Key;

typedef enum {
    SecCrypto_Data_Type_CERT = SecCrypto_Alg_Key_ALL_KEY_ALG_MAX, // 14
    SecCrypto_Data_Type_OTH,
    SecCrypto_Data_Type_MAX
} SecCrypto_Data_Type;

typedef enum {
    SecCrypto_Data_ID_INVALID = -1,
    SecCrypto_Data_ID_CA = 0,
    SecCrypto_Data_ID_PKI_AUTH_PUB,
    SecCrypto_Data_ID_MAX
} SecCrypto_Data_ID;

typedef enum {
    SecCrypto_KeyID_SYM_KEY_FBE = 0x400, // used for FBE
    SecCrypto_KeyID_ASYM_KEY_1024_xx, //128 * 3(n, e, d)
    SecCrypto_KeyID_ASYM_KEY_2048, //256 * 3(n, e, d)
    SecCrypto_KeyID_ASYM_KEY_3072, //384 * 3(n, e, d)
    SecCrypto_KeyID_ASYM_KEY_4096, //512 * 3(n, e, d)
    SecCrypto_KeyID_ECC_KEY_P192, //24 + 48 = 72
    SecCrypto_KeyID_ECC_KEY_P256, //32 + 64 = 96
    SecCrypto_KeyID_ECC_KEY_P384, //48 + 96 = 144
    SecCrypto_KeyID_ECC_KEY_P521, //66 + 132 = 198
    SecCrypto_KeyID_ECC_KEY_E521, //66 + 132 = 198
    SecCrypto_KeyID_ECC_KEY_SM2, //32 + 64 = 96
} SecCrypto_KeyID;


typedef enum {
    SecCrypto_Cipher_Mode_ECB = 0,
    SecCrypto_Cipher_Mode_CBC = 1,
    SecCrypto_Cipher_Mode_CTR = 2,
    SecCrypto_Cipher_Mode_CFB = 3,
    SecCrypto_Cipher_Mode_OFB = 4,
    SecCrypto_Cipher_Mode_CCM = 5,
    SecCrypto_Cipher_Mode_GCM = 6,
    SecCrypto_Cipher_Mode_XTS = 7,
    SecCrypto_Cipher_Mode_CMAC = 8,
    SecCrypto_Cipher_Mode_MAX,
} SecCrypto_Cipher_Mode;

typedef enum {
    SecCrypto_Cipher_OP_ENC = 0,
    SecCrypto_Cipher_OP_DEC,
} SecCrypto_Cipher_OP;

typedef enum {
    SecCrypto_RSA_Type_PUB      = 0x0,/**< public key */
    SecCrypto_RSA_Type_PRI      = 0x1 /**< private key */
} SecCrypto_RSA_Type;

typedef enum {
    SecCrypto_RSA_Padding_NONE      = 0x0,/**< No padding */
    SecCrypto_RSA_Padding_OAEP      = 0x1,/**< Optimal Asymmetric Encryption Padding */
    SecCrypto_RSA_Padding_EME_PKCS  = 0x2,/**< EME-PKCS padding */
    SecCrypto_RSA_Padding_EMSA_PKCS = 0x3,/**< EMSA-PKCS padding, equal openssl dgst padding,pkcs#1 v1.5 */
    SecCrypto_RSA_Padding_PSS       = 0x4 /**< EMSA-PSS padding */
} SecCrypto_RSA_Padding;

typedef enum {
    SecCrypto_Status_FAILED = 0xFFFF,
    SecCrypto_Status_PANIC,
    SecCrypto_Status_IS_NOT_BE_INIT,

    SecCrypto_Status_SUCCESS = 0x00,

    SecCrypto_Status_INIT_ERROR = 0x10,
    SecCrypto_Status_RPMSG_ESTABLISH_ERROR,
    SecCrypto_Status_PRMSG_DONT_INIT,

    SecCrypto_Status_RPMSG_MSG_SIZE_TOO_BIG,
    SecCrypto_Status_RPMSG_SEND_ERROR,
    SecCrypto_Status_RPMSG_RECEIVE_ERROR,

    SecCrypto_Status_INVALID_PARAs = 0x11,
    SecCrypto_Status_INVALID_API = 0x22,
    SecCrypto_Status_NOT_SUPPORT = 0x33,
    SecCrypto_Status_INVALID_ALGO = 0x44,
    SecCrypto_Status_ECDSA_SIGN_INNER_ERROR = 0x55,
    SecCrypto_Status_ECDSA_VERIFY_INNER_ERROR = 0x66,
    SecCrypto_Status_CONTEXT_ID_EXISTED = 0x77,
    SecCrypto_Status_CONTEXT_ID_NOT_EXISTED = 0x88,
    SecCrypto_Status_CONTEXT_POOL_FULLED = 0x99,
    SecCrypto_Status_BUFFER_SMALL = 0xAA,
    SecCrypto_Status_UNALIGNED_DATA = 0xBB,
    SecCrypto_Status_INVALID_ADDR = 0xCC,
    SecCrypto_Status_CE_BUSY = 0xDD,
    SecCrypto_Status_INVALID_KEY_ID = 0xEE,

    SecCrypto_Status_SHAREMEM_INIT_ERROR = 0x20,
    SecCrypto_Status_SHAREMEM_POOL_FULLED,
    SecCrypto_Status_SHAREMEM_ERROR,
    SecCrypto_Status_SHAREMEM_BE_USED,
    SecCrypto_Status_SHAREMEM_BLOCK_SMALL,
    SecCrypto_Status_SHAREMEM_DEINIT_ERROR,
    SecCrypto_Status_SHAREMEM_WR_DATA_TOO_LARGE,

    SecCrypto_Status_KEY_SIZE_ERROR      = 0x30,
    SecCrypto_Status_NO_ENTRY      = 0x0100,

} SecCrypto_Status;

typedef enum {
    // SecCrypto_SecID_CCM_CA = 1,                 // pki ca cert
    // SecCrypto_SecID_CCM_PKI_AUTH_PUB = 2,       // pki public for auth
    // SecCrypto_SecID_CCM_UPDATE_ENC = 3,         // update AES encrypt key
    // SecCrypto_SecID_CCM_UPDATE_PAYLOAD_PUB = 4, // update public key
    // SecCrypto_SecID_CCM_PKI_CCM_CERT = 5,       // ccm cert
    // SecCrypto_SecID_CCM_PKI_CCM_PRI = 6,        // ccm private key
    // SecCrypto_SecID_CCM_COMM_PAYLOAD_PUB = 7,   // SM2 public key for encrypt communcation playload
    // SecCrypto_SecID_CCM_COMM_PAYLOAD_PRI = 8,   // SM2 private key for decrypt communcation playload
    // SecCrypto_SecID_CCM_COMM_APPKEY = 9,        // app key used for hash the ommuncation header
    // SecCrypto_SecID_CCM_FOTA_ABI_PUB = 10,        // verify the componet.iso
    SecCrypto_SecID_CCM_CA = 1,
    SecCrypto_SecID_CCM_GENERAL_CERT_P12 = 2,
    SecCrypto_SecID_CCM_GENERAL_KEY  = 3,
    SecCrypto_SecID_CCM_UPDATE_ENC   = 4,
    SecCrypto_SecID_CCM_UPDATE_PAYLOAD_PUB = 5,
    SecCrypto_SecID_CCM_DIAG_AUTH_KEY = 6,
    SecCrypto_SecID_CCM_CERT = 7,
    SecCrypto_SecID_CCM_KEY  = 8,

} SecCrypto_SecID;

typedef enum {
    SecCrypto_DataID_SGW_UPDATE_ENC = 256,
    SecCrypto_DataID_SGW_UPDATE_PAYLOAD_PUB = 257,
    SecCrypto_DataID_SGW_DIAG_AUTH_KEY = 258,
    SecCrypto_DataID_SGW_GENERAL_CERT_P12 = 259,

    SecCrypto_DataID_SGW_ROOT_CERT = 280,
    SecCrypto_DataID_SGW_GENERAL_CERT = 281,
    SecCrypto_DataID_SGW_GENERAL_KEY = 282,
    SecCrypto_DataID_SGW_GENERAL_MID_CERT = 283,
    SecCrypto_DataID_SGW_DEV_CERT = 284,
    SecCrypto_DataID_SGW_DEV_KEY  = 285,
    SecCrypto_DataID_SGW_DEV_MID_CERT = 286,
    SecCrypto_DataID_SGW_OLD_DEV_CERT = 287,
    SecCrypto_DataID_SGW_OLD_DEV_KEY = 288,
    SecCrypto_DataID_SGW_OLD_DEV_MID_CERT = 289,
} SecCrypto_DataID;

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _SECURITY_CRYPTO_TYPES_H_ */


