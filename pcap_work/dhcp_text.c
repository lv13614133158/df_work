// dhcp_text.c
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

// DHCP头部结构 - 和接收端保持一致
struct dhcp_packet {
    uint8_t op;          // 操作码
    uint8_t htype;       // 硬件地址类型
    uint8_t hlen;        // 硬件地址长度
    uint8_t hops;        // 跳数
    uint32_t xid;        // 事务ID
    uint16_t secs;       // 秒数
    uint16_t flags;      // 标志
    uint8_t ciaddr[4];   // 客户端IP
    uint8_t yiaddr[4];   // 你的IP
    uint8_t siaddr[4];   // 服务器IP
    uint8_t giaddr[4];   // 网关IP
    uint8_t chaddr[16];  // 客户端硬件地址
    uint8_t sname[64];   // 服务器名称
    uint8_t file[128];   // 启动文件名
    uint32_t magic_cookie;// 魔数cookie
    uint8_t options[32]; // DHCP选项
};

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    struct dhcp_packet packet;  // 直接使用结构体，而不是字符数组
    
    // 创建UDP套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        return -1;
    }
    
    // 设置广播选项
    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        return -1;
    }
    
    // 配置服务器地址
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(67);
    servaddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    
    // 初始化DHCP包
    memset(&packet, 0, sizeof(packet));
    packet.op = 1;  // BOOTP请求
    packet.htype = 1;  // 以太网
    packet.hlen = 6;   // MAC地址长度
    packet.hops = 0;
    packet.xid = htonl(0x12345678);  // 固定事务ID
    packet.secs = 0;
    packet.flags = htons(0x8000);  // 广播标志
    
    // 设置客户端MAC地址 (只需要前6个字节)
    memcpy(packet.chaddr, "\x00\x11\x22\x33\x44\x55\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
    
    // 设置服务器名和文件名为空
    memset(packet.sname, 0, 64);
    memset(packet.file, 0, 128);
    
    // 关键修复：直接设置结构体中的magic_cookie字段
    packet.magic_cookie = htonl(0x63825363);  // DHCP魔数cookie
    
    // 设置DHCP选项
    packet.options[0] = 53;  // DHCP消息类型选项
    packet.options[1] = 1;   // 长度
    packet.options[2] = 1;   // Discover消息类型
    packet.options[3] = 255; // 结束选项
    
    printf("发送DHCP Discover包以测试重放攻击检测...\n");
    printf("魔数cookie值: 0x%08x\n", packet.magic_cookie);
    printf("包大小: %zu 字节\n", sizeof(packet));
    
    // 快速发送多个相同的包来模拟重放攻击 (更短的时间间隔)
    for (int i = 0; i < 100; i++) {
        int sent = sendto(sockfd, &packet, sizeof(packet), 0,
                         (struct sockaddr*)&servaddr, sizeof(servaddr));
        if (sent < 0) {
            perror("sendto failed");
        } else {
            printf("发送包 %d, 发送字节数: %d\n", i+1, sent);
        }
        usleep(50000);  // 50ms间隔，更容易触发重放攻击检测
    }
    
    close(sockfd);
    return 0;
}