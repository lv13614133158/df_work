
#include "openAes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void main()
{

    char * key = "wq23456789abcde1";
    char * iv = "wq23456789abcde1";
    char * plaintext = "0123456789abcdef111";
    char * ciphertext;
    int rt = 1;
    int ase_len;
    char buff[128]={0};
    rt=aes_init(key,iv);
    if(rt != 0 )
    {
        printf("aes_init error error = %d \n",rt);
    }
    printf("old plaintext = %s \n",plaintext);
    printf("\n");
    ciphertext = aes_char_write(plaintext);
    printf("ciphertext = %s \n",ciphertext);
    printf("\n");

    ase_len = strlen(ciphertext);
    plaintext = aes_char_read(ciphertext,128);
    printf("new plaintext = %s \n",plaintext);


    aes_delete();
    if(plaintext)
    {
        free(plaintext);
    }
    if (ciphertext) {
        free(ciphertext);
    }

}