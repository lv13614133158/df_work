//
// Created by tangxx on 9/24/19.
//

#ifndef CHIMASAILDDEMO_CRYPTOGRAM_H
#define CHIMASAILDDEMO_CRYPTOGRAM_H
#include <stdint.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _cypher_t{
	int len_data;
	uint8_t data[0];
}cypher_t;



// set work path
int set_work_directory(const char *dir);

/*
*	@brief Storage key and IV vector
*	@param <mode>,Key storage method, 1 :Store to file, 2:Store to white box...
			<data> Data to be stored (if IV not exist,the data length is 16byte,else the data length is 32byte string
	@return  success return keyIndex,failure return NULL
*/


char *set_value(int mode,const char *data);


int get_value(int mode,const char *keyIndex,char *outbuf);
int delete_value(const char *data);

void getIndexKeyNetWork(char* _input);
void getIndexSN(char* _input);
int  getMode(void);
void setIndexKey(const char *indexKey);
void setIndexSN(const char *indexSN);
void setMode(int mode);

/*
*	@brief aes cbc encrypt ,
*	@param: plain :Plain text object
*			key: 16-byte string
*			iv:16-byte string or NULL
*	@return :Successfully returned ciphertext cypher_t* structure,failed returned NULL
*			note(Return the cypher_t *structure, you need to manually release the memory at the end)
*/

cypher_t *aes_cbc_encrypt(cypher_t* data_in,const char *key,const char *iv);

/*
*	@brief aes cbc decrypt ,
*	@param: cypeher :Ciphertext object
*			key: 16-byte string
*			iv:16-byte string or NULL
*	@return :Successfully returned ciphertext cypher_t* structure,failed returned NULL
*			note(Return the cypher_t *structure, you need to manually release the memory at the end)
*/
cypher_t* aes_cbc_decrypt(cypher_t* cypehr,const char *key,const char *iv);

/*
*	@brief aes ecb encrypt ,
*	@param: plain :Plain text object
*			key: 16-byte string
*	@return :Successfully returned ciphertext cypher_t* structure,failed returned NULL
*			note(Return the cypher_t *structure, you need to manually release the memory at the end)
*/
cypher_t* aes_ecb_encrypt(cypher_t* plain,const char *key);

/*
*	@brief aes ecb decrypt ,
*	@param: plain :Plain text object
*			key: 16-byte string
*	@return :Successfully returned ciphertext cypher_t* structure,failed returned NULL
*			note(Return the cypher_t *structure, you need to manually release the memory at the end)
*/
cypher_t* aes_ecb_decrypt(cypher_t* cypher,const char *key);


#ifdef __cplusplus
}  /* end of the 'extern "C"' block */
#endif

#endif //CHIMASAILDDEMO_CRYPTOGRAM_H
