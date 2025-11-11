#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <libwebsockets.h>
#include <unistd.h>

static volatile int force_exit = 0;
static int message_count = 0;
static int use_ssl = 0;  // 标记是否使用 SSL

// 专门测试 WRITEABLE 回调的函数
static int callback_websocket_client(struct lws *wsi, enum lws_callback_reasons reason,
                                   void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("[连接] WebSocket 连接已建立\n");
            // 连接建立后触发第一次写入
            lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE: {
            printf("[写入回调] LWS_CALLBACK_CLIENT_WRITEABLE 被调用\n");
            
            // 准备发送缓冲区
            unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 256];
            char message[128];
            
            // 构造消息内容
            snprintf(message, sizeof(message), "消息 #%d: 当前时间戳 %ld", 
                    ++message_count, time(NULL));
            
            printf("[写入回调] 准备发送: %s\n", message);
            
            // 复制消息到发送缓冲区（注意 LWS 的缓冲区要求）
            memcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING], message, strlen(message));
            
            // 发送消息
            int result = lws_write(wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], 
                                  strlen(message), LWS_WRITE_TEXT);
            
            if (result < 0) {
                printf("[写入回调] 发送失败\n");
                force_exit = 1;
            } else {
                printf("[写入回调] 成功发送 %d 字节\n", result);
            }
            
            // 每隔几次写入后等待一段时间再触发下一次写入
            if (message_count < 5) {
                printf("[写入回调] 2秒后触发下一次写入...\n");
                sleep(2);
                // 这里可能会在 SSL 失败时导致问题
                lws_callback_on_writable(wsi);
            } else {
                printf("[写入回调] 已发送5条消息，程序即将退出\n");
                force_exit = 1;
            }
            break;
        }

        case LWS_CALLBACK_CLIENT_RECEIVE:
            printf("[接收] 收到服务器响应: %.*s\n", (int)len, (char *)in);
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            printf("[错误] 连接错误: %s\n", in ? (char *)in : "未知错误");
            force_exit = 1;
            break;

        case LWS_CALLBACK_CLOSED:
            printf("[关闭] 连接已关闭\n");
            force_exit = 1;
            break;

        default:
            // 不处理其他回调
            break;
    }

    return 0;
}

// 协议定义
static struct lws_protocols protocols[] = {
    {
        "test-protocol",
        callback_websocket_client,
        0,
        4096,
    },
    { NULL, NULL, 0, 0 } // 结束标记
};

// 信号处理函数
static void sigint_handler(int sig) {
    force_exit = 1;
}

int main(int argc, char **argv) {
    struct lws_context_creation_info info;
    struct lws_context *context;
    struct lws *wsi;
    struct lws_client_connect_info ccinfo;
    

    
    // 注册信号处理器
    signal(SIGINT, sigint_handler);

    // 初始化 context 创建信息
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    // 创建 libwebsockets 上下文
    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "错误: 无法创建 libwebsockets 上下文\n");
        return -1;
    }

    printf("已创建 libwebsockets 上下文\n");

    // 初始化客户端连接信息
    memset(&ccinfo, 0, sizeof(ccinfo));
    ccinfo.context = context;
    

        // HTTP 连接 - 正常工作
        ccinfo.address = "ws.ifelse.io";        // HTTP 测试服务器
        ccinfo.port = 80;                       // HTTP 端口
        ccinfo.ssl_connection = 0;              // 不使用 SSL

    
    ccinfo.path = "/";
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = protocols[0].name;

    printf("正在连接到 WebSocket 服务器 %s:%d (SSL: %s)...\n", 
           ccinfo.address, ccinfo.port, use_ssl ? "是" : "否");

    // 建立连接
    wsi = lws_client_connect_via_info(&ccinfo);
    if (!wsi) {
        fprintf(stderr, "错误: 无法连接到服务器\n");
        lws_context_destroy(context);
        return -1;
    }

    printf("连接请求已发送，等待服务器响应...\n");

    // 主循环 - 专门观察 WRITEABLE 回调行为
    printf("进入主事件循环，观察 LWS_CALLBACK_CLIENT_WRITEABLE 回调\n");
    printf("程序将尝试发送5条消息后退出\n");
    printf("----------------------------------------\n");
    
    while (!force_exit) {
        lws_service(context, 100); // 100ms 超时
    }

    printf("----------------------------------------\n");
    printf("测试完成，共发送 %d 条消息\n", message_count);
    
    // 清理资源
    lws_context_destroy(context);
    printf("已释放 libwebsockets 上下文\n");

    return 0;
}