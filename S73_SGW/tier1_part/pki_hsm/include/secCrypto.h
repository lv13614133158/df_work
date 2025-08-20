#ifndef _SECURITY_CRYPTO_H_
#define _SECURITY_CRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "secCryptoType.h"
// #include "hsm-rpmsg/Hsm_Types.h"

uint32_t secCryptoInit(void);
uint32_t secCryptoDeinit(void);
uint32_t secCryptoInitStorage(void);
uint32_t secCryptoDeinitStorage(void);
uint32_t secCryptoDefaultKeyImport(void);
//==========================random=======================================
/*
** @brief: get the random streaming
** @param: dst <- the output address
**         dst_size -> the random len which must be multiple * 16, like 16,32,48,
** @ret:   0: success; other failed.
*/
uint32_t secCryptoGetRnd(uint8_t *dst, uint32_t dst_size);

//==========================hash=======================================
/*
** @brief: calculate the hash, not support now
** @param: hash_mode -> special the hash mode. ref enum SecCrypto_Alg_Hash
           src -> special the souce address
**         src_size -> the souce len
           dst -> special the output result address
**         dst_size -> the result len
** @ret:   0: success; other failed.
*/
uint32_t secCryptoHash(SecCrypto_Alg_Hash hash_mode, const uint8_t *src, uint32_t src_size, uint8_t *dst, uint32_t dst_size);
/*
** @brief: calculate the hash for big data with sequence(start update finish)
** @param: job_id <- special the sequce ID for the coninue caluation
           hash_mode -> special the hash mode. ref enum SecCrypto_Alg_Hash
** @ret:   0: success; other failed.
*/
uint32_t secCryptoHashStart(uint32_t *job_id, SecCrypto_Alg_Hash hash_mode);
uint32_t secCryptoHashUpdate(uint32_t job_id, const uint8_t *src, uint32_t src_size);
uint32_t secCryptoHashFinish(uint32_t job_id, uint8_t *dst, uint32_t dst_size);

/*
** @brief: calculate the hash
** @param: hash_mode -> special the hash mode. ref enum SecCrypto_Alg_Hash
           src -> special the souce address
**         src_size -> the souce len
           dst -> special the output result address
**         dst_size -> the result len
** @ret:   0: success; other failed.
*/
uint32_t secCryptoHashFile2Str(SecCrypto_Alg_Hash hash_mode, const char *srcFile, uint8_t *dst, uint32_t dst_size);
uint32_t secCryptoHashFile2File(SecCrypto_Alg_Hash hash_mode, const char *srcFile, const char *dstFile);

//==========================signature=======================================
/*
** @brief: calculate the cmac
** @param: key_id -> special the key index from key storage.
           alg_key -> special the key mode of cmac. ref enum SecCrypto_Alg_Key
           src -> special the souce address
**         src_size -> the souce len
           dst -> special the output result address
**         dst_size -> the result len
** @ret:   0: success; other failed.
*/
uint32_t secCryptoCmac(uint16_t key_id,
                            SecCrypto_Alg_Key alg_key,
                            const uint8_t *src, uint32_t src_size,
                            uint8_t *dst, uint32_t dst_size);
/*
** @brief: calculate the hmac
** @param: key_id -> special the key index from key storage.
           alg_key -> fixed SecCrypto_Alg_Key_SYM_KEY
           hash_mode -> special the hash mode. ref enum SecCrypto_Alg_Hash
           src -> special the souce address
**         src_size -> the souce len
           dst -> special the output result address
**         dst_size -> the result len
** @ret:   0: success; other failed.
*/
uint32_t secCryptoHmac(uint16_t key_id,
                            SecCrypto_Alg_Key alg_key,
                            SecCrypto_Alg_Hash hash_mode,
                            const uint8_t *src, uint32_t src_size,
                            uint8_t *dst, uint32_t dst_size);
uint32_t secCryptoCmacVerify(uint16_t key_id,
                                  SecCrypto_Alg_Key alg_key,
                                  const uint8_t *src1, uint32_t src1_size,
                                  const uint8_t *src2, uint32_t src2_size);
uint32_t secCryptoHmacVerify(uint16_t key_id,
                                  SecCrypto_Alg_Key alg_key, SecCrypto_Alg_Hash hash_mode,
                                  const uint8_t *src1, uint32_t src1_size,
                                  const uint8_t *src2, uint32_t src2_size);
/*
** @brief: calculate the hmac
** @param: key_id -> special the key index from key storage.
           alg_key -> fixed SecCrypto_Alg_Key_SYM_KEY
           hash_mode -> special the hash mode. ref enum SecCrypto_Alg_Hash
           src -> special the souce address
**         src_size -> the souce len
           dst -> special the output result address
**         be_param -> the big-endian of the src, 1:big endian; 0: little endian
** @ret:   0: success; other failed.
*/
uint32_t secCryptoRsaSign(uint16_t key_id,
                               SecCrypto_Alg_Key alg_key,
                               const uint8_t *src, uint32_t src_size,
                               uint8_t *dst, uint32_t dst_size,
                               SecCrypto_RSA_Padding padding_type,
                               SecCrypto_Alg_Hash hash_mode,
                               bool be_param);
uint32_t secCryptoRsaSignVerify(uint16_t key_id,
                                     SecCrypto_Alg_Key alg_key,
                                     const uint8_t *src1, uint32_t src1_size,
                                     const uint8_t *src2, uint32_t src2_size,
                                     SecCrypto_RSA_Padding padding_type,
                                     SecCrypto_Alg_Hash hash_mode,
                                     bool be_param);

uint32_t secCryptoEcdsaSign(uint16_t key_id,
                                 SecCrypto_Alg_Key alg_key,
                                 const uint8_t *src, uint32_t src_size,
                                 uint8_t *dst, uint32_t dst_size,
                                 SecCrypto_Alg_Hash hash_mode);
uint32_t secCryptoEcdsaSignVerify(uint16_t key_id,
                                       SecCrypto_Alg_Key alg_key,
                                       const uint8_t *src1, uint32_t src1_size,
                                       const uint8_t *src2, uint32_t src2_size,
                                       SecCrypto_Alg_Hash hash_mode);

uint32_t secCryptoSm2Sign(uint16_t key_id,
                               SecCrypto_Alg_Key alg_key,
                               const uint8_t *src, uint32_t src_size,
                               uint8_t *dst, uint32_t dst_size);
uint32_t secCryptoSm2SignVerify(uint16_t key_id,
                                    SecCrypto_Alg_Key alg_key,
                                    const uint8_t *src1, uint32_t src1_size,
                                    const uint8_t *src2, uint32_t src2_size);

//==========================crypto=======================================
uint32_t secCryptoAesEnc(uint32_t key_id, SecCrypto_Cipher_Mode mode,
                             uint8_t *iv, uint32_t iv_size,
                             uint8_t *msg, uint32_t msg_size,
                             uint8_t *cip, uint32_t cip_size);
uint32_t secCryptoAesDec(uint32_t key_id, SecCrypto_Cipher_Mode mode,
                             uint8_t *iv, uint32_t iv_size,
                             uint8_t *msg, uint32_t msg_size,
                             uint8_t *cip, uint32_t cip_size);
uint32_t secCryptoAesStart(uint32_t *job_id, uint16_t key_id,
                       SecCrypto_Alg_Key alg_key,
                       SecCrypto_Cipher_Mode mode, SecCrypto_Cipher_OP cipher_op,
                       const uint8_t *iv, uint32_t iv_size);
uint32_t secCryptoAesUpdate(uint32_t job_id,
                        const uint8_t *src, uint32_t src_size,
                        uint8_t *dst, uint32_t dst_size);
uint32_t secCryptoAesFinish(uint32_t job_id);

uint32_t secCryptoAead(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                       SecCrypto_Cipher_Mode mode, SecCrypto_Cipher_OP cipher_op,
                       const uint8_t *nonce, uint32_t nonce_size,
                       const uint8_t *aad, uint32_t aad_size,
                       const uint8_t *src, uint32_t src_size,
                       uint8_t *dst, uint32_t dst_size,
                       uint8_t *tag, uint32_t tag_size);
uint32_t secCryptoAeadGcmEnc(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                         const uint8_t *nonce, uint32_t nonce_size,
                         const uint8_t *aad, uint32_t aad_size,
                         const uint8_t *src, uint32_t src_size,
                         uint8_t *dst, uint32_t dst_size,
                         uint8_t *tag, uint32_t tag_size);
uint32_t secCryptoAeadGcmEncFile(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                         const uint8_t *nonce, uint32_t nonce_size,
                         const uint8_t *aad, uint32_t aad_size,
                         const uint8_t *src, uint32_t src_size,
                         const char *dstFile,
                         const char *tagFile);
uint32_t secCryptoAeadGcmEncFile2(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                         const uint8_t *nonce, uint32_t nonce_size,
                         const uint8_t *aad, uint32_t aad_size,
                         const char *srcFile,
                         const char *dstFile,
                         const char *tagFile);
uint32_t secCryptoAeadGcmEncDefault(uint8_t sec_id, const uint8_t *src, uint32_t src_size);
uint32_t secCryptoAeadGcmDec(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                         const uint8_t *nonce, uint32_t nonce_size,
                         const uint8_t *aad, uint32_t aad_size,
                         const uint8_t *src, uint32_t src_size,
                         const uint8_t *srctag, uint32_t srctag_size,
                         uint8_t *dst, uint32_t dst_size);
// need delete *dst
uint32_t secCryptoAeadGcmDecFile(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                         const uint8_t *nonce, uint32_t nonce_size,
                         const uint8_t *aad, uint32_t aad_size,
                         const char *src_file,
                         const char *srctag_file,
                         uint8_t **dst, uint32_t* dst_size);
uint32_t secCryptoAeadGcmDecFile2(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                         const uint8_t *nonce, uint32_t nonce_size,
                         const uint8_t *aad,uint32_t aad_size,
                         const char *src_file,
                         const char *srctag_file,
                         const char *dstFile);
uint32_t secCryptoAeadGcmDecDefault(uint8_t sec_id, uint8_t **src, uint32_t* src_size);
uint32_t secCryptoRsaEnc(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                              const uint8_t *src, uint32_t src_size,
                              const uint8_t *dst, uint32_t *dst_size,
                              SecCrypto_RSA_Padding padding_type,
                              SecCrypto_Alg_Hash hash_mode,
                              bool be_param);
uint32_t secCryptoRsaDec(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                              const uint8_t *src, uint32_t src_size,
                              const uint8_t *dst, uint32_t *dst_size,
                              SecCrypto_RSA_Padding padding_type,
                              SecCrypto_Alg_Hash hash_mode,
                              bool be_param);

uint32_t secCryptoSm4Start(uint32_t *job_id, uint16_t key_id,
                                SecCrypto_Alg_Key alg_key,
                                SecCrypto_Cipher_Mode mode, SecCrypto_Cipher_OP cipher_op,
                                const uint8_t *iv, uint32_t iv_size);
uint32_t secCryptoSm4Update(uint32_t job_id,
                                 const uint8_t *src, uint32_t src_size,
                                 uint8_t *dst, uint32_t dst_size);
uint32_t secCryptoSm4Finish(uint32_t job_id);

//==========================key manager=======================================
uint32_t secCryptoKeyGenerate(uint16_t *key_id, SecCrypto_Alg_Key alg_key);
uint32_t secCryptoKeyUpdate(uint16_t key_id, SecCrypto_Alg_Key alg_key);
uint32_t secCryptoKeyDelete(uint16_t key_id, SecCrypto_Alg_Key alg_key);
uint32_t secCryptoKeyInstall(uint16_t *key_id, SecCrypto_Alg_Key alg_key,
                                  const uint8_t *key, uint32_t key_size);

/*
** @brief: calculate the hmac
** @param: key_id -> special the key index from key storage.
           alg_key -> special the key mode of cmac. ref enum SecCrypto_Alg_Key
           hash_mode -> special the hash mode. ref enum SecCrypto_Alg_Hash
           src -> special the souce address
**         src_size -> the souce len
           dst -> special the output result address
**         xkey -> xkey is used for xts mode; other NULL
** @ret:   0: success; other failed.
*/
uint32_t secCryptoKeySymInstall(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                                    const uint8_t *key, const uint8_t *xkey, uint32_t key_size);
/*
** @brief: install the RSA key, only support RSA key format, not x509 format
** @param: key_id -> special the key index from key storage.
           alg_key -> special the key mode of cmac. ref enum SecCrypto_Alg_Key
           hash_mode -> special the hash mode. ref enum SecCrypto_Alg_Hash
           n -> special the souce address
**         e -> the souce len
           d -> used for private key.
** @ret:   0: success; other failed.
*/
uint32_t secCryptoKeyRsaInstall(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                                     const uint8_t *n, const uint8_t *e, const uint8_t *d, uint32_t n_size);
uint32_t secCryptoKeyRsaInstallFile(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                                         const char *file_name, SecCrypto_RSA_Type rsa_type);
uint32_t secCryptoKeyEcdsaInstall(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                                       uint8_t *x, uint8_t *y, uint8_t *d, uint32_t size);
// uint32_t secCryptoKeyUninstall(uint16_t key_id, SecCrypto_Alg_Key alg_key);

//uint32_t secCryptoDataSave(uint16_t *data_id, const uint8_t *data, uint32_t data_size);
uint32_t secCryptoDataSave(uint16_t data_id, const uint8_t *data, uint32_t data_size);
uint32_t secCryptoDataGetsize(uint16_t data_id, uint32_t *data_size);
uint32_t secCryptoDataGet(uint16_t data_id, uint8_t *data, uint32_t data_size);
uint32_t secCryptoDataUpdate(uint16_t data_id, const uint8_t *data, uint32_t data_size);
uint32_t secCryptoDataDelete(uint16_t data_id);


uint32_t secCryptoGetcarnd(uint16_t key_id, SecCrypto_Alg_Key alg_key, uint8_t *dst);

uint32_t secCryptoKeySymInstall(uint16_t key_id, SecCrypto_Alg_Key alg_key,
                                    const uint8_t *key, const uint8_t *xkey, uint32_t key_size);

uint32_t secCryptoKeySymUpdate(uint16_t key_id, SecCrypto_Alg_Key alg_key, const uint8_t *key, const uint8_t *xkey, uint32_t key_size);

uint32_t secCryptoRsaUpdate(uint16_t key_id, SecCrypto_Alg_Key alg_key, const uint8_t *n, const uint8_t *e, const uint8_t *d, uint32_t n_size);

uint32_t secCryptoKeyEcdsaUpdate(uint16_t key_id, SecCrypto_Alg_Key alg_key, const uint8_t *x, const uint8_t *y, const uint8_t *d, uint32_t size);

uint32_t secCryptokeyRsaGetPubkey(uint16_t key_id, SecCrypto_Alg_Key alg_key, uint32_t key_size, const uint8_t *n, const uint8_t *e);

uint32_t secCryptoKeyEccGetPubkey(uint16_t key_id, SecCrypto_Alg_Key alg_key, uint32_t key_size, const uint8_t *x, const uint8_t *y);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _SECURITY_CRYPTO_H_ */
