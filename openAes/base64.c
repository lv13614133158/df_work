
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "base64.h"
static const unsigned char base64_table[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * base64_encode - Base64 encode
 * @src: Data to be encoded
 * @len: Length of the data to be encoded
 * @out_len: Pointer to output length variable, or %NULL if not used
 * Returns: Allocated buffer of out_len bytes of encoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer. Returned buffer is
 * nul terminated to make it easier to use as a C string. The nul terminator is
 * not included in out_len.
 */
unsigned char * base64_encode(const unsigned char *src, size_t len,
			      size_t *outlen)
{
	size_t olen;
	unsigned char *output;
	olen = (size_t)(len * 4 / 3 + 4); /* 3-byte blocks to 4-byte */
	olen += olen / 72; /* line feeds */
	olen++; /* nul termination */
	if (olen < len)
		return NULL; /* integer overflow */
	output = (unsigned char*)malloc(olen);
	if (output == NULL)
		return NULL;
	memset(output,0,olen);
	const char *base64_tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        int off = 0;
        int i,mod;
 
        mod = len % 3;
        for(i = 0; i < (len - mod); i+= 3)
        {
                output[off++] = base64_tbl[(src[i] >> 2) & 0x3F];//i [7,2]
                output[off++] = base64_tbl[( ((src[i] & 0x3) << 4) | (src[i+1] >> 4) ) & 0x3F];//i [1,0] i+1 [7,4]
                output[off++] = base64_tbl[( ((src[i+1] & 0xF) << 2) | (src[i+2] >> 6) ) & 0x3F];//i+1 [3,0] i+2 [7,6]
                output[off++] = base64_tbl[src[i+2] & 0x3F];//i+2 [5,0]
        }
 
        if(mod == 1){
                output[off++] = base64_tbl[(src[i] >> 2) & 0x3F];//i [7,2]
                output[off++] = base64_tbl[(src[i] & 0x3) << 4];
                output[off++] = '=';
                output[off++] = '=';
        }
        else if(mod == 2){
                output[off++] = base64_tbl[(src[i] >> 2) & 0x3F];//i [7,2]
                output[off++] = base64_tbl[( ((src[i] & 0x3) << 4) | (src[i+1] >> 4) ) & 0x3F];//i [1,0] i+1 [7,4]
                output[off++] = base64_tbl[(src[i+1] & 0xF) << 2];
                output[off++] = '=';
        }
        output[off] = '\0';
        if(NULL != outlen){
                *outlen = off;
        }
        return output;
        
}


/**
 * base64_decode - Base64 decode
 * @src: Data to be decoded
 * @len: Length of the data to be decoded
 * @out_len: Pointer to output length variable
 * Returns: Allocated buffer of out_len bytes of decoded data,
 * or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */

unsigned char * base64_decode(const unsigned char *src, size_t len,
			      size_t *out_len)
{
	unsigned char dtable[256], *out, *pos, in[4], block[4], tmp;
	size_t i, count, olen;

	memset(dtable, 0x80, 256);
	for (i = 0; i < sizeof(base64_table) - 1; i++)
		dtable[base64_table[i]] = (unsigned char) i;
	dtable['='] = 0;

	count = 0;
	for (i = 0; i < len; i++) {
		if (dtable[src[i]] != 0x80)
			count++;
	}

	if (count == 0 || count % 4)
		return NULL;

	olen = count / 4 * 3;
	pos = out = malloc(olen);
	if (out == NULL)
		return NULL;

	count = 0;
	for (i = 0; i < len; i++) {
		tmp = dtable[src[i]];
		if (tmp == 0x80)
			continue;

		in[count] = src[i];
		block[count] = tmp;
		count++;
		if (count == 4) {
			*pos++ = (block[0] << 2) | (block[1] >> 4);
			*pos++ = (block[1] << 4) | (block[2] >> 2);
			*pos++ = (block[2] << 6) | block[3];
			count = 0;
		}
	}

	if (pos > out) {
		if (in[2] == '=')
			pos -= 2;
		else if (in[3] == '=')
			pos--;
	}

	*out_len = pos - out;
	return out;
}
