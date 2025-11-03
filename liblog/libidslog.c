#include "libidslog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/liblog_socket"

// 回调函数类型定义
typedef void (*log_message_callback)(const char* message, int len);

// 函数声明
void* receiver_thread_func(void* arg);
void* client_handler(void* arg);
void ids_log_internal_cleanup(void);
void cleanup_handler(int sig);
void ignore_sigpipe(void);

static int log_initialized = 0;
static int socket_fd = -1;
static int client_fd = -1;  // 客户端连接文件描述符
static log_message_callback message_callback = NULL;

// 用于接收消息的线程
static pthread_t receiver_thread;
static int receiver_running = 0;
static int receiver_thread_created = 0;

// 自动清理标志
static int auto_cleanup_enabled = 0;

// 信号处理标志
static int signal_handlers_registered = 0;

/**
 * 忽略SIGPIPE信号
 */
void ignore_sigpipe(void) {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGPIPE, &sa, NULL);
}

/**
 * 信号处理函数
 */
void cleanup_handler(int sig) {
    static int cleaned_up = 0;
    if (!cleaned_up) {
        cleaned_up = 1;
        ids_log_reader_cleanup();
        ids_log_cleanup();
    }
    
    if (sig == SIGINT || sig == SIGTERM) {
        signal(sig, SIG_DFL);
        raise(sig);
    }
}

/**
 * 注册信号处理函数
 */
static void register_signal_handlers(void) {
    if (!signal_handlers_registered) {
        signal(SIGINT, cleanup_handler);
        signal(SIGTERM, cleanup_handler);
        signal_handlers_registered = 1;
    }
}

/**
 * 确保套接字存在
 * @return 0 成功，-1 失败
 */
static int ensure_socket_exists(void)
{
    struct stat st;
    
    // 检查套接字是否已经存在
    if (stat(SOCKET_PATH, &st) == 0) {
        // 如果是套接字文件，可以继续使用
        if (S_ISSOCK(st.st_mode)) {
            return 0;
        } else {
            fprintf(stderr, "Path %s exists but is not a socket\n", SOCKET_PATH);
            return -1;
        }
    }
    
    // 套接字不存在，将在创建时自动创建
    return 0;
}

/**
 * 初始化日志系统（写入端）
 * @return 0 成功，-1 失败
 */
int ids_log_init(void)
{
    struct sockaddr_un addr;
    
    // 注册信号处理函数
    register_signal_handlers();
    
    // 忽略SIGPIPE信号
    ignore_sigpipe();
    
    // 确保套接字路径可用
    if (ensure_socket_exists() != 0) {
        return -1;
    }
    
    // 创建套接字
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("socket creation failed");
        return -1;
    }
    
    // 设置套接字地址
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    // 尝试连接到服务器（非阻塞）
    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        if (errno != ECONNREFUSED && errno != ENOENT) {
            perror("connect to socket failed");
            close(client_fd);
            client_fd = -1;
            return -1;
        }
        // 如果是连接被拒绝或文件不存在，我们允许初始化成功
        // 实际的连接将在第一次写入时处理
    }
    
    log_initialized = 1;
    return 0;
}

/**
 * 初始化日志系统（读取端/服务器端）
 * @param callback 接收消息时调用的回调函数
 * @param enable_auto_cleanup 是否启用自动清理
 * @return 0 成功，-1 失败
 */
int ids_log_reader_init(log_message_callback callback, int enable_auto_cleanup)
{
    struct sockaddr_un addr;
    
    if (callback == NULL) {
        return -1;
    }
    
    // 注册信号处理函数
    register_signal_handlers();
    
    // 确保套接字路径可用
    if (ensure_socket_exists() != 0) {
        return -1;
    }
    
    message_callback = callback;
    auto_cleanup_enabled = enable_auto_cleanup;
    
    // 创建套接字
    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket creation failed");
        return -1;
    }
    
    // 设置套接字地址
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    
    // 删除可能存在的旧套接字文件
    unlink(SOCKET_PATH);
    
    // 绑定套接字
    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind socket failed");
        close(socket_fd);
        socket_fd = -1;
        return -1;
    }
    
    // 开始监听连接
    if (listen(socket_fd, 5) == -1) {
        perror("listen failed");
        close(socket_fd);
        unlink(SOCKET_PATH);
        socket_fd = -1;
        return -1;
    }
    
    // 创建接收线程
    receiver_running = 1;
    receiver_thread_created = 0;
    
    if (pthread_create(&receiver_thread, NULL, receiver_thread_func, NULL) != 0) {
        perror("pthread_create failed");
        close(socket_fd);
        unlink(SOCKET_PATH);
        socket_fd = -1;
        return -1;
    }
    
    receiver_thread_created = 1;
    return 0;
}

/**
 * 接收消息的线程函数
 */
void* receiver_thread_func(void* arg)
{
    while (receiver_running && socket_fd != -1) {
        struct sockaddr_un client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // 接受客户端连接
        int client_socket = accept(socket_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            if (receiver_running) {
                perror("accept failed");
            }
            continue;
        }
        
        // 为每个客户端连接创建一个处理线程
        pthread_t client_thread;
        int* client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = client_socket;
        
        if (pthread_create(&client_thread, NULL, client_handler, client_socket_ptr) != 0) {
            perror("pthread_create failed for client");
            free(client_socket_ptr);
            close(client_socket);
        } else {
            // 分离线程以便自动回收资源
            pthread_detach(client_thread);
        }
    }
    
    return NULL;
}

/**
 * 客户端处理函数
 */
void* client_handler(void* arg)
{
    int client_socket = *(int*)arg;
    free(arg);
    
    char buffer[1024];
    int bytes_read;
    
    // 读取客户端发送的数据
    while (receiver_running) {
        bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            
            // 处理末尾的换行符，避免重复
            int actual_len = bytes_read;
            while (actual_len > 0 && (buffer[actual_len-1] == '\n' || buffer[actual_len-1] == '\r')) {
                actual_len--;
                buffer[actual_len] = '\0';
            }
            
            if (message_callback != NULL && receiver_running) {
                message_callback(buffer, actual_len);
            }
        } else if (bytes_read == -1) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                if (receiver_running) {
                    // perror("read error");
                }
                break;
            }
            usleep(10000); // 休眠10ms
        } else if (bytes_read == 0) {
            // 客户端关闭连接
            break;
        }
    }
    
    close(client_socket);
    return NULL;
}

/**
 * 写入日志数据并通过套接字通知外部程序
 * @param data 日志数据
 * @param len 数据长度
 * @return 写入的字节数，失败返回-1
 */
int ids_log_write(const char *data, int len)
{
    if (!log_initialized) {
        // 如果未初始化，尝试初始化
        if (ids_log_init() != 0) {
            return -1;
        }
    }
    
    if (data == NULL || len <= 0) {
        return -1;
    }

    if (len > 1023) {  
        fprintf(stderr, "Message too long: %d bytes (max 1023)\n", len);
        return -1;
    }
    // 检查文件描述符是否有效，如果无效尝试重新连接
    if (client_fd == -1) {
        // 重新初始化连接
        if (ids_log_init() != 0) {
            return -1;
        }
    }
    
    // 如果连接仍然无效，说明没有reader，可以选择舍弃消息或返回错误
    if (client_fd == -1) {
        // 没有有效的连接，舍弃消息并返回错误
        return -1;
    }
    
    int total_written = 0;
    
    if (client_fd != -1) {
        // 不再自动添加换行符，保持原始数据格式
        ssize_t result = write(client_fd, data, len);
        
        if (result == -1) {
            // 检查是否是因为没有读取端
            if (errno == EPIPE) {
                // 管道破裂，关闭连接但不退出程序
                close(client_fd);
                client_fd = -1;
                // 返回错误表示消息未发送
                return -1;
            } else {
                perror("write to socket failed");
                // 关闭并重新打开连接
                close(client_fd);
                client_fd = -1;
                return -1;
            }
        }
        
        total_written = (int)result;
    } else {
        // 没有连接，舍弃消息
        return -1;
    }
    
    return total_written;
}

/**
 * 清理写入端资源
 */
void ids_log_cleanup(void)
{
    if (client_fd != -1) {
        close(client_fd);
        client_fd = -1;
    }
    
    log_initialized = 0;
}

/**
 * 清理读取端资源
 */
void ids_log_reader_cleanup(void)
{
    receiver_running = 0;
    
    // 关闭服务器套接字
    if (socket_fd != -1) {
        close(socket_fd);
        socket_fd = -1;
        unlink(SOCKET_PATH);
    }
    
    // 等待线程结束
    if (receiver_thread_created) {
        // 给线程一些时间来清理
        usleep(200000); // 等待200ms
        receiver_thread_created = 0;
    }
}

/**
 * 检查是否有活动的写入端
 * @return 1 有活动写入端，0 无活动写入端
 */
int ids_log_has_writer(void)
{
    // 在套接字实现中，这个功能需要额外的机制来实现
    // 可以通过尝试连接来简单检查
    int test_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (test_fd >= 0) {
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
        
        int result = connect(test_fd, (struct sockaddr*)&addr, sizeof(addr));
        close(test_fd);
        return (result == 0) ? 1 : 0;
    }
    return 0;
}