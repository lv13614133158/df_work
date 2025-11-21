#ifndef IDS_WEBSOCKET_H
#define IDS_WEBSOCKET_H
#include <libwebsockets.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "cJSON.h"
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <pthread.h>

typedef struct ids_websocketInfoModult{
   char url[128];
   char channelId[128];
   char equmentType[128];
   char sn[128];
   int  version;
   int  sslShutdown;
   int  port;
   int  intervalread;
   int  intervalheart;
   int  intervalkey;
   char listPath[128];
   int  caShutdown;
   char ca_certs_Path[128];
   char client_certs_Path[128];
   char client_key_Path[128];
	char* ca_cert_mem;
	size_t ca_cert_mem_len;
	char* client_cert_mem;
	size_t client_cert_mem_len;
	char* client_key_mem;
	size_t client_key_mem_len;

}ids_websocketInfoModult_t;

static ids_websocketInfoModult_t websocketInfoModult;
#define wbsGetUrl()      websocketInfoModult.url
#define wbsGetEqument()  websocketInfoModult.equment
#define wbsGetChannelId()websocketInfoModult.channelId
#define wbsGetSn()       websocketInfoModult.sn
#define wbsGetSsl()      websocketInfoModult.sslShutdown
#define wbsGetCa()       websocketInfoModult.caShutdown
#define wbsGetPort()     websocketInfoModult.port
#define wbsGetCaCertPath()     websocketInfoModult.ca_certs_Path
#define wbsGetClientCertPath()     websocketInfoModult.client_certs_Path
#define wbsGetClientKeyPath()     websocketInfoModult.client_key_Path
#define wbsGetCacertmem()     websocketInfoModult.ca_cert_mem
#define wbsGetClientcertmem()     websocketInfoModult.client_cert_mem
#define wbsGetClientkeymem()     websocketInfoModult.client_key_mem
#define wbsGetCacertmemlen()     websocketInfoModult.ca_cert_mem_len
#define wbsGetClientcertmemlen()     websocketInfoModult.client_cert_mem_len
#define wbsGetClientkeymemlen()     websocketInfoModult.client_key_mem_len
#define KEY_SAVE_MAX  128
typedef struct ids_keystore{
	char m_manageKey[KEY_SAVE_MAX];
	char m_sessionKey[KEY_SAVE_MAX];
	char m_sn[KEY_SAVE_MAX];
	char m_token[512];
}ids_keystore_t;
static ids_keystore_t keystore;
#define wbGet_manageKey()      keystore.m_manageKey
#define wbGet_sessionKey()     keystore.m_sessionKey
#define wbGet_sn()      keystore.m_sn
#define wbGet_token()      keystore.m_token

static int CallBack(struct lws *wsi, enum lws_callback_reasons reason,void *user, void *in, size_t len);

int websocketclient_init(ids_websocketInfoModult_t * websocketInfoModult,ids_keystore_t *keystore);
void websocketclient_delete(void);
int SendStrData(const char* str);
#endif

