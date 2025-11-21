#include "idswebsocket.h"


static struct lws_context *context;
static char server_address[256] = {0}, *pro = "lws-minimal";
static int interrupted = 0, port = 7681, ssl_connection = 0;
struct lws *wsisend = NULL;
static bool s_reinit_wbs_connect = false;
static pthread_t ws_thread;
static const struct lws_protocols protocols[] = {
		{ "lws-minimal", CallBack, 4096, 4096, 0, NULL, 0 },
		{ NULL, NULL, 0, 0 }
};
static int wbsClient_localWebSocketclient();
static int ConnectClient(void);
static int wbsClient_init(void);
// websocket回调 参数：上下文管理、回调事件、自定结构体、输入数据，输入长度
static int CallBack(struct lws *wsi, enum lws_callback_reasons reason,
		void *user, void *in, size_t len)
{
	int  logLen = 255;
	char spdlog[255] ={0};
    //printf("DEBUG: websocket Callback reason: %d\n", reason); // 添加这一行
	switch (reason) { 
		case LWS_CALLBACK_CLIENT_ESTABLISHED:
            wsisend = wsi;
            s_reinit_wbs_connect = false;
			printf("WebSocket 连接成功\n");
            break;
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		case LWS_CALLBACK_CLOSED:
        case LWS_CALLBACK_CLIENT_CLOSED:
            s_reinit_wbs_connect = true;
			wsisend = NULL;
			printf("WebSocket 连接已关闭\n");
            break;
		default:
			break;
	}
	return 0;
}


int websocketclient_init(ids_websocketInfoModult_t * _websocketInfoModult,ids_keystore_t *_keystore)
{

    websocketInfoModult = *_websocketInfoModult;
	keystore = *_keystore;
   
	int ret = pthread_create(&ws_thread, NULL, wbsClient_localWebSocketclient, NULL);
	if (ret != 0) {
		fprintf(stderr, "Error: Failed to create WebSocket thread.\n");
		return -1; 
	}
	pthread_detach(ws_thread);
	return 0;
}



static int ConnectClient(void)
{
	char *tempToken = wbGet_token();
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
		sprintf(buff,"/live/v1.5/tbox/linux?sn=%s&token=%s",wbGet_sn(),token+1);
		//printf("%d   %s  %s \n",0,"idps_websocket",buff);
		i.path   = buff;
		i.host   = i.address;
		i.origin = i.address;
		i.ssl_connection = ssl_connection;
		i.protocol = pro;
		i.local_protocol_name = pro;
		//usleep(connection_delay);
		char spdlog[255] ={0};
		snprintf(spdlog, 255, "%s: %s:%d connecting...\n", __func__, i.address, i.port);
		//printf("%d   %s  %s \n",0,"idps_websocket", spdlog);
		if (!(wsisend = lws_client_connect_via_info(&i))) {
			return 1;
		}
	}
	return 0;
}

static int wbsClient_init(void)
{
	int ca_chose = wbsGetCa();	
	int ssl_chose = wbsGetSsl();	
	struct lws_context_creation_info info;
	int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;//LLL_INFO | LLL_DEBUG
	lws_set_log_level(logs, NULL);
	memset(&info, 0, sizeof info);
	info.options   = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	info.port      = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
	info.protocols = protocols;    //注册回调函数

	if(ca_chose)
	{		
			info.client_ssl_ca_mem = wbsGetCacertmem();
            info.client_ssl_ca_mem_len = wbsGetCacertmemlen();
            info.client_ssl_cert_mem = wbsGetClientcertmem();
            info.client_ssl_cert_mem_len = wbsGetClientcertmemlen();
            info.client_ssl_key_mem = wbsGetClientkeymem();
            info.client_ssl_key_mem_len = wbsGetClientkeymemlen();
	}else{
			info.client_ssl_ca_filepath = wbsGetCaCertPath() ;
			info.client_ssl_cert_filepath = wbsGetClientCertPath();
			info.client_ssl_private_key_filepath = wbsGetClientKeyPath();
	}


	if(ssl_chose == 1)
		ssl_connection |= LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | \
						 LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK |    \
						 LCCSCF_ALLOW_INSECURE | LCCSCF_ALLOW_EXPIRED;

	info.fd_limit_per_thread = (unsigned int)(1 + 1 + 1);//1 + clients + 1
	context = lws_create_context(&info);  
	if (!context) {
		printf("%d   %s  %s \n",4,"idps_websocket", "lws init failed\n");
		return -1;
	}
	return 0;
}

static int wbsClient_localWebSocketclient()
{
    int n = 0;

    if (-1 == wbsClient_init()) {
        return 1;
    }
    
    while(!interrupted) {
        if(!wsisend && context) {
            ConnectClient();
        }

        if (context) {
            n = lws_service(context, 100000);
        } else {
            sleep(1);
        }
        
        if (s_reinit_wbs_connect == true) {
            if (context) {
                lws_context_destroy(context);
                context = NULL;
            }
            wbsClient_init();
            printf("%d   %s  %s \n", 0, "idps_websocket", "wbsClient_init");
            s_reinit_wbs_connect = false;
        }
    }

    char spdlog[255] = {0};
    snprintf(spdlog, 255, "exiting service loop. n = %d, interrupted = %d\n", n, interrupted);
    printf("%d   %s  %s \n", 0, "idps_websocket", spdlog);

    if (context) {
        lws_context_destroy(context);
        context = NULL;
    }
    return 0;
}

void websocketclient_delete(void)
{
    interrupted = 1;
}

int SendStrData(const char* str)
{
    if (!str) {
        return -1;
    }
    if (!wsisend || !context) {
        printf("WebSocket connection is not ready\n");
        return -2;
    }
    if (lws_get_socket_fd(wsisend) < 0) {
        printf("WebSocket socket is invalid\n");
        return -2;
    }
    char* message = NULL;
    int m = 0, n = 0;
    message = (char*)malloc(strlen(str) + LWS_PRE);
    if (!message) {
        return -1;
    }
    memset(message, 0, strlen(str) + LWS_PRE);
    memcpy(&message[LWS_PRE], str, strlen(str));
	if(!wsisend)return -1;

    m = lws_write(wsisend, message + LWS_PRE, strlen(str), LWS_WRITE_TEXT);
    n = strlen(str);
    free(message);
    if (m < n || m < 0) {
        printf("发送数据失败 lws_write err %d\n",m);
        return -3;
    }
    return 0;
}