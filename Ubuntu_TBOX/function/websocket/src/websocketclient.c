/* *
 *  socket:注册->连接->回调--->|-------------接收------------->|normal----|正常，删除数据库里相应内容
 *          ^                |                			|--->|rpc       |错误，再次将数据库里对应内容加到发送队列
 *          |                |                                 |
 *     将回调函数等参数传入     发送<-----------|                  |---->回复rpcack,rpcres
 *                          				|     
 * 			产生事件---------------------->发送队列
 *              |                           ^
 *              |------------->数据库--------|
 *                                      发送队列为空时
 * */
#include <libwebsockets.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "cJSON.h"
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include "spdloglib.h"
#include "websocketclient.h"
#include "websocketnormal.h"
#include "websocketConfig.h"
#include "websocketUtils.h"
#include "websocketLoop.h"
#include "websocketrpc.h"
#include "websocketTool.h"
#include <sys/stat.h>
#include "ConfigParse.h"

// websocket 连接部分
int lstagerun = websocketstage0;
static int message_delay    = 500000; // microseconds
static int connection_delay = 100000; // microseconds
static struct lws_context *context;
static char server_address[256] = {0}, *pro = "lws-minimal";
static int interrupted = 0, port = 7681, ssl_connection = 0;
struct lws *wsisend = NULL;
static bool s_reinit_wbs_connect = false;
static bool s_wbs_connection_succ = false;

// 接收数据处理
#define MAXLISTSIZE   (512)

static list listwebsockettx;
static list listwebsocketrx;
static list listwebsocketrpc;
static list listwebsocketnormal;
static pthread_mutex_t    request_tx_lock     = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t    request_rx_lock     = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t    request_rpc_lock    = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t    request_normal_lock = PTHREAD_MUTEX_INITIALIZER;
sem_t listnotifymessage;
sem_t listnotifyrpc;
sem_t listnotifynormal;

#define websocketfree()	sem_destroy(&listnotifymessage);\
						sem_destroy(&listnotifyrpc);\
						sem_destroy(&listnotifynormal);\
						list_destroy(&listwebsockettx);\
						list_destroy(&listwebsocketrx);\
						list_destroy(&listwebsocketrpc);\
						list_destroy(&listwebsocketnormal);

#define websocketnew()	sem_init(&listnotifymessage,0,0);\
						sem_init(&listnotifyrpc,0,0);\
						sem_init(&listnotifynormal,0,0);\
						list_init(&listwebsockettx,free);\
						list_init(&listwebsocketrx,free);\
						list_init(&listwebsocketrpc,free);\
						list_init(&listwebsocketnormal,free);

#define websocketthread()  	pthread_t threadHandle = 0,threadRpc = 0,threadNormal = 0;\
							int stacksize = 5*1024*1024;\
							pthread_attr_t attr;\
							int ret = pthread_attr_init(&attr);\
							if((ret = pthread_attr_setstacksize(&attr, stacksize)) != 0){\
								log_v("idps websocket","statcksize set error for webmessage process");\
							}\
							pthread_create(&(threadHandle),&attr,ProcessMessageThread,NULL);\
							if((ret = pthread_attr_destroy(&attr)) != 0)\
								log_v("idps_websocket","thread attr destory error for webmessage process\n");\

// 调试链表
void DBGlist(list *listTx)
{
	int i = 0;
	list_elmt* node = listTx->head;
	do{
		printf("list size:%d\n", listTx->size);
		if(node) {
			printf("i:%d,%s\n", i++,node->data);
			node = node->next;
		}
	}while(node);
} 

// 接收数据分发						  							
static void *ProcessMessageThread(void *args)
{
	int s = 0;
	struct timespec ts;
    list_elmt* node = NULL;
	enum webProcessType enumtype = OTHERS;
	pthread_detach(pthread_self());
	while(!interrupted)
	{
    	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        	perror("clock_gettime");
    	ts.tv_sec += 1;
		while ((s = sem_timedwait(&listnotifymessage, &ts)) == -1 && errno == EINTR)
        	continue;       /* Restart if interrupted by handler */

		node = NULL;
		pthread_mutex_lock(&request_rx_lock);
        node = list_rem_head(&listwebsocketrx);  
		pthread_mutex_unlock(&request_rx_lock);
		if(node)
		{
			enumtype = wbsDispatchData(node->data);
			printf("事件类型：%d data:%s\n",enumtype, node->data);
			switch(enumtype)
			{
			case NORMAL:
				wbsNormalProcess(node->data);				
				break;
			case RPC:
				wbsRpcProcess(node->data);			
				break;
			case OTHERS:
				printf("handle other***\n");
				break;
				default:
				break;
			}
			if(node->data)
				free(node->data);
			free(node);
		}
	}
}

// 事件直接放数据库和发送队列
void wbsClient_localWebsocketSendEx(char* _input){

    int listsize = 0;

	pthread_mutex_lock(&request_tx_lock);
	listsize = list_size(&listwebsockettx);
	if (listsize > 1024)
	{
		/*clear list*/
		log_d("Send list", "listwebsockettx list overflow! clear!");
		while (listsize > 0)
		{
			list_elmt* node = list_rem_head(&listwebsockettx); 
			listsize = list_size(&listwebsockettx);
			if (node)
			{
				if (node->data)
				{
					free(node->data);
					node->data = NULL;
				}
				free(node);
				node = NULL;
			}
		}
	}
	list_ins_next(&listwebsockettx,NULL,_input);
	pthread_mutex_unlock(&request_tx_lock);
	if(wsisend)
		lws_callback_on_writable(wsisend);	
}

static void wbsClient_localWebsocketSend(long long lseqnumber, char *data, bool send_only_once)
{
	cJSON *json = cJSON_Parse(data);

	if (json)
	{
		
		#if 1
		cJSON *item_body = cJSON_GetObjectItemCaseSensitive(json, "body");
		if (item_body) {
			char* bodyString = cJSON_GetStringValue(item_body);
			cJSON *item_new_body = cJSON_Parse(bodyString);
			if(item_new_body)
			{
				cJSON *item_latitude = cJSON_GetObjectItemCaseSensitive(item_new_body, "latitude");
				if (item_latitude != NULL) {
					cJSON_SetIntValue(item_latitude, 0); 
				}
				cJSON *item_longitude = cJSON_GetObjectItemCaseSensitive(item_new_body, "longitude");
				if (item_longitude != NULL) {
					cJSON_SetIntValue(item_longitude, 0); 
				}
				if(item_body->valuestring != NULL)
				{
					free(item_body->valuestring);
					item_body->valuestring = NULL;
				}
				item_body->valuestring = cJSON_PrintUnformatted(item_new_body);
				cJSON_Delete(item_new_body); 
				bodyString=NULL;
			}
		}
		
		#endif

		char *print_data = cJSON_PrintUnformatted(json);
		if (print_data)
		{
			if (!wsisend && send_only_once == 1)
			{
				log_d("Not add Map", print_data);
				free(print_data);
				cJSON_Delete(json);
				free(data);
				return;
			}
			else
			{
				log_d("Add Map", print_data);
			}
			free(print_data);
		}
		cJSON_Delete(json);
	}

	wbsAddMap(lseqnumber, data, send_only_once);
	if(wsisend)
		lws_callback_on_writable(wsisend);
}

void wbsClient_localWebsocketSend_no_network(char* _input)
{
	log_d("offline", _input);
}

void wbsClient_sendEventData(char* _postid,char* _data,char* _event)
{
	if(!_data)return;
	long long lseqnumber=0;
	char* s = wbsGetformEvent(_data, _postid, &lseqnumber);
	if (networkFunctionEnabled())
	{
		wbsClient_localWebsocketSend(lseqnumber, s, 0);
	}
	else
	{
		wbsClient_localWebsocketSend_no_network(s);
		if(s)free(s);
	}
}

void wbsClient_sendInfoData(char* _data)
{
	if(!_data)return;
	long long lseqnumber=0;
	char* s = wbsGetformInfo(_data, &lseqnumber);
	if (networkFunctionEnabled())
	{
		wbsClient_localWebsocketSend(lseqnumber, s, 0);
	}
	else
	{
		wbsClient_localWebsocketSend_no_network(s);
		if(s)free(s);
	}
}

void wbsClient_sendHeartBeat()
{
	long long lseqnumber=0;
	char* s = wbsGetHeartBeat(&lseqnumber);
	if (networkFunctionEnabled())
	{
		/*hearBeat don't need to resend*/
		wbsClient_localWebsocketSend(lseqnumber, s, 1);
	}
	else
	{
		wbsClient_localWebsocketSend_no_network(s);
		if(s)free(s);
	}
}

void wbsClient_sendRpcAck(long long lseqnumber)
{
	char* s = wbsGetRpcAck(lseqnumber);
	if (networkFunctionEnabled())
	{
		/*don't need to resend*/
		wbsClient_localWebsocketSend(lseqnumber, s, 1);
	}
	else
	{
		wbsClient_localWebsocketSend_no_network(s);
		if(s)free(s);
	}
}

void wbsClient_sendRpcResp(char* _data)
{
	if(!_data)return;
	long long lseqnumber=0;
	char* s = wbsGetRpcResp(_data, &lseqnumber);
	if (networkFunctionEnabled())
	{
		/*don't need to resend*/
		wbsClient_localWebsocketSend(lseqnumber, s, 1);
	}
	else
	{
		wbsClient_localWebsocketSend_no_network(s);
		if(s)free(s);
	}
}

// 数据库里数据添加到发送队列
void wbsClient_readLoopData()
{
	wbsSetRetrySend();
	if(wsisend)
		lws_callback_on_writable(wsisend);
}

// 数据接收
static int ReceiveData(char *in)
{
	log_d("SOCKET receive",in);
	char* message = NULL;
	message = (char*)malloc(strlen(in)+1);
	bzero(message,strlen(in)+1);
	memcpy(message,in,strlen(in));

	pthread_mutex_lock(&request_rx_lock);
	list_ins_next(&listwebsocketrx,NULL,message);
	pthread_mutex_unlock(&request_rx_lock);	
	sem_post(&listnotifymessage);
	return 0;
}

// 数据发送队列发送
static int SendData(struct lws *wsi)
{
	int listsize = 0;
	char* message = NULL;
	int m= 0, n = 0;
	int i = 0, ret = 0;
	map_elmt map_buff = {0};

	for(int i = 0; i < WBSMAP_MAX; i++)
	{
		ret = wbsGetMap(i, &map_buff);
		if (ret < 0)
		{
			continue;
		}

		cJSON *json = cJSON_Parse(map_buff.data);
		if(!json)return -1;
		char* _lstring = cJSON_PrintUnformatted(json);
		if(json)cJSON_Delete(json);
	
		message = (char*)malloc(strlen(_lstring) + LWS_PRE);
		bzero(message,strlen(_lstring) + LWS_PRE);
		memcpy(&message[LWS_PRE],_lstring,strlen(_lstring));
		m = lws_write(wsi,message + LWS_PRE,strlen(_lstring),LWS_WRITE_TEXT);
		free(message);
		message = NULL;
	
		n = strlen(_lstring);
		if(m < n)
		{
			char spdlog[255] = {0};
			snprintf(spdlog, 255, "%d < %d :%s", m, n, spdlog);
			log_e("SOCKET send failed", spdlog);
		}
	
		if(_lstring)free(_lstring);
		if (map_buff.data)
		{
			free(map_buff.data);
			map_buff.data = NULL;
		}
		if (map_buff.send_only_once)
		{
			wbsDelMap(map_buff.key);
		}
	}

	return 0;
}

// websocket回调 参数：上下文管理、回调事件、自定结构体、输入数据，输入长度
static int CallBack(struct lws *wsi, enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	int  logLen = 255;
	char spdlog[255] ={0};

	//snprintf(spdlog, logLen, "websocket callback called with reason %d", reason);
	//log_v("idps_websocket",spdlog);
	switch (reason) 
	{
	case LWS_CALLBACK_PROTOCOL_INIT:
		log_v("idps_websocket","websocket callbak init");
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		s_wbs_connection_succ = false;
		snprintf(spdlog, logLen, "websocket connection err: %s\n", in ? (char *)in :"(null)");
		log_e("idps_websocket",spdlog);
        wsisend = NULL;
		break;
	
	case LWS_CALLBACK_CLIENT_ESTABLISHED: // 第一次建立连接
		s_wbs_connection_succ = true;
		lws_callback_on_writable(wsi);
		log_d("idps_websocket","++++websocket connection established\n");
		break;

	case LWS_CALLBACK_CLIENT_CLOSED:
		s_wbs_connection_succ = false;
		log_d("idps_websocket","----websocket connection closed\n");
		wsisend = NULL;
		break;
	
	case LWS_CALLBACK_CLIENT_WRITEABLE: // 发送数据
	   	SendData(wsi);
		break;

	case LWS_CALLBACK_TIMER:
		lws_callback_on_writable(wsi);
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE: // 接收数据
		ReceiveData(in);
		break;

	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		snprintf(spdlog, logLen, "server initiated connection close: len = %lu, in = %s\n",
		 (unsigned long)len, (char*)in);
		log_v("idps_websocket",spdlog);

	default:
		break;
	}
	return 0;
	//return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static const struct lws_protocols protocols[] = {
		{ "lws-minimal", CallBack, 4096, 4096, 0, NULL, 0 },
		{ NULL, NULL, 0, 0 }
};

int wbsClient_set_reinit(bool reinit)
{
	s_reinit_wbs_connect = reinit;

	while (s_reinit_wbs_connect == true)
	{
		printf("s_reinit_wbs_connect\n");
		sleep(1);
	}
	return 0;
}

// websocket连接
static int ConnectClient(void)
{
	char *tempToken = wbsGetTokennumber();
	port = wbsGetPort();
	memcpy(server_address,wbsGetUrl(),strlen(wbsGetUrl()));
	if((*tempToken) != 0)
	{
		struct lws_client_connect_info i;
		memset(&i, 0, sizeof(i));
		i.context = context;
		i.port    = port;
		i.address = server_address;
		char buff[1024] = {0};
		char *token = strchr(tempToken,' ');
		sprintf(buff,"/live/v1.5/tbox/linux?sn=%s&token=%s",wbsGetSnnumber(),token+1);
		log_v("idps_websocket",buff);
		i.path   = buff;
		i.host   = i.address;
		i.origin = i.address;
		i.ssl_connection = ssl_connection;
		i.protocol = pro;
		i.local_protocol_name = pro;
		//usleep(connection_delay);
		char spdlog[255] ={0};
		snprintf(spdlog, 255, "%s: %s:%d connecting...\n", __func__, i.address, i.port);
		log_v("idps_websocket", spdlog);
		if (!(wsisend = lws_client_connect_via_info(&i))) {
			return 1;
		}
	}
	return 0;
}

int wbsClient_init(void)
{
	int ssl_chose = wbsGetSsl();
	struct lws_context_creation_info info;
	int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;//LLL_INFO | LLL_DEBUG
	char *client_cert_buff = NULL;
	int client_cert_len = 0;
	char *client_private_key_buff = NULL;
	int client_private_key_len = 0;
	char *root_cert_buff = NULL;
	int root_cert_len = 0;

	/*init certificate buff*/
	if (get_pki_root_cert())
	{
		root_cert_len = strlen(get_pki_root_cert());
		root_cert_buff = malloc(root_cert_len);
		if (root_cert_buff)
		{
			memcpy(root_cert_buff, get_pki_root_cert(), root_cert_len);
		}
	}

	if (get_pki_client_cert())
	{
		client_cert_len = strlen(get_pki_client_cert());
		client_cert_buff = malloc(client_cert_len);
		if (client_cert_buff)
		{
			memcpy(client_cert_buff, get_pki_client_cert(), client_cert_len);
		}
	}

	if (get_pki_client_private_key())
	{
		client_private_key_len = strlen(get_pki_client_private_key());
		client_private_key_buff = malloc(client_private_key_len);
		if (client_private_key_buff)
		{
			memcpy(client_private_key_buff, get_pki_client_private_key(), client_private_key_len);
		}
	}
	lws_set_log_level(logs, NULL);
	memset(&info, 0, sizeof info);
	info.options   = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	info.port      = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
	info.protocols = protocols;

	

	/*Method 1. load the certificate in memory*/
	if (root_cert_buff)
	{
		info.client_ssl_ca_filepath = NULL;  // 不使用文件方式，改为内存块方式
		info.client_ssl_ca_mem = root_cert_buff;  // 将CA证书作为内存块传入
		info.client_ssl_ca_mem_len = root_cert_len; // 内存块大小
 	}

	if (client_cert_buff)
	{
		info.client_ssl_cert_filepath = NULL;  // 不使用文件方式，改为内存块方式
		info.client_ssl_cert_mem = client_cert_buff;  // 将客户端证书作为内存块传入
		info.client_ssl_cert_mem_len = client_cert_len; // 内存块大小
 	}

	if (client_private_key_buff)
	{
		info.client_ssl_private_key_filepath = NULL;  // 不使用文件方式，改为内存块方式
		info.client_ssl_key_mem = client_private_key_buff;	// 将客户端私钥作为内存块传入
		info.client_ssl_key_mem_len = client_private_key_len; // 内存块大小
 	}
	
#if 0
	info.client_ssl_ca_filepath = get_pki_root_cert();//如果服务器有CA证书则需要这行代码
	info.client_ssl_cert_filepath = get_pki_client_cert();
	info.client_ssl_private_key_filepath = get_pki_client_private_key();
#endif
	if(ssl_chose == 1)

	ssl_connection |= LCCSCF_USE_SSL;
	info.fd_limit_per_thread = (unsigned int)(1 + 1 + 1);//1 + clients + 1
	context = lws_create_context(&info);


	if (root_cert_buff)
	{
		free(root_cert_buff);
		root_cert_buff = NULL;
	}
	if (client_cert_buff)
	{
		free(client_cert_buff);
		client_cert_buff = NULL;
	}
	if (client_private_key_buff)
	{
		free(client_private_key_buff);
		client_private_key_buff = NULL;
	}
	if (!context) {
		log_e("idps_websocket", "lws init failed\n");
		return -1;
	}

	return 0;
}

// websocket建立客户端
int wbsClient_localWebSocketclient()
{
	int n = 0;
	wbsClient_initPosition();
	websocketnew()
    websocketthread()

	if (-1 == wbsClient_init())
	{
		//return 1;
	}
	
	while(!interrupted){
		if(!wsisend){
			if (context)
			{
				ConnectClient();
				log_v("idps_websocket", "ConnectClient end");
			}
		}

		if (context)
		{
			n = lws_service(context, 100000);
		}
		else
		{
			sleep(1);
		}

		//log_v("idps_websocket", "lws_service end");
		if (s_reinit_wbs_connect == true)
		{
			if (context)
			{
				lws_context_destroy(context);
				context = NULL;
			}
			wbsClient_init();
			log_v("idps_websocket", "wbsClient_init");
			s_reinit_wbs_connect = false;
		}
	}

	char spdlog[255]={0};
	snprintf(spdlog, 255, "exiting service loop. n = %d, interrupted = %d\n", n, interrupted);
	log_v("idps_websocket",spdlog);

	if (context)
	{
		lws_context_destroy(context);
		context = NULL;
	}
	websocketfree()
	return 0;
}


// websocket停止
void wbsClient_localStopWbClient(void)
{
	wbsClient_endPosition();
	interrupted = 1;
}

bool wbs_client_connect_success(void)
{
	return s_wbs_connection_succ;
}

