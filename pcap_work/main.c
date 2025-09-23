#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define DHCP_TABLE_SIZE 256
#define BOOTP_REQUEST 1
#define BOOTP_REPLY   2
#define DHCP_OPTIONS_MAGIC_COOKIE 0x63825363

// DHCP事务信息结构
typedef struct {
    u_long xid;              // 事务ID
    u_char chaddr[6];        // 客户端MAC地址
    time_t first_seen;       // 第一次见到该事务的时间
    time_t last_seen;        // 最后一次见到该事务的时间
    int count;               // 该事务出现的次数
} dhcp_transaction_info;

// 全局变量
dhcp_transaction_info dhcp_table[DHCP_TABLE_SIZE];
int dhcp_table_size = 0;

// DHCP头部结构
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

// 清理过期的DHCP事务记录
void cleanup_dhcp_table(time_t current_time) {
    int i, j;
    for (i = 0; i < dhcp_table_size; i++) {
        if (current_time - dhcp_table[i].last_seen > 300) {
            for (j = i; j < dhcp_table_size - 1; j++) {
                memcpy(&dhcp_table[j], &dhcp_table[j+1], sizeof(dhcp_transaction_info));
            }
            dhcp_table_size--;
            i--;
        }
    }
}

// 检查是否为DHCP重放攻击 - 基于速率的检测
int is_dhcp_replay_attack(struct dhcp_packet *dhcp_pkt, time_t current_time) {
    int i;
    int found = -1;
    u_long xid = ntohl(dhcp_pkt->xid);
    
    // 查找是否已存在该事务ID和MAC地址的记录
    for (i = 0; i < dhcp_table_size; i++) {
        if (dhcp_table[i].xid == xid && 
            memcmp(dhcp_table[i].chaddr, dhcp_pkt->chaddr, 6) == 0) {
            found = i;
            break;
        }
    }
    
    if (found != -1) {
        // 更新记录
        dhcp_table[found].last_seen = current_time;
        dhcp_table[found].count++;
        
        // 基于速率的检测：如果在1秒内收到超过3个相同请求，则认为是攻击
        if (current_time - dhcp_table[found].last_seen <= 1) {
            if (dhcp_table[found].count >= 3) {
                // 重置计数器以允许持续检测
                dhcp_table[found].count = 0;
                return 1;
            }
        } else {
            // 如果时间间隔太长，重置计数器
            dhcp_table[found].count = 1;
        }
    } else {
        // 添加新记录
        if (dhcp_table_size < DHCP_TABLE_SIZE) {
            dhcp_table[dhcp_table_size].xid = xid;
            memcpy(dhcp_table[dhcp_table_size].chaddr, dhcp_pkt->chaddr, 6);
            dhcp_table[dhcp_table_size].first_seen = current_time;
            dhcp_table[dhcp_table_size].last_seen = current_time;
            dhcp_table[dhcp_table_size].count = 1;
            dhcp_table_size++;
        }
    }
    
    return 0;
}

// 处理DHCP数据包
void process_dhcp_packet(const u_char *packet, int packet_len) {
    struct ether_header *eth_header;
    struct ip *ip_header;
    struct udphdr *udp_header;
    struct dhcp_packet *dhcp_pkt;
    time_t current_time = time(NULL);
    
    // 解析以太网头部
    eth_header = (struct ether_header *)packet;
    if (ntohs(eth_header->ether_type) != ETHERTYPE_IP) return;

    // 解析IP头部
    ip_header = (struct ip *)(packet + sizeof(struct ether_header));
    if (ip_header->ip_p != IPPROTO_UDP) return;

    // 解析UDP头部
    udp_header = (struct udphdr *)(packet + sizeof(struct ether_header) + (ip_header->ip_hl * 4));
    
    // 检查是否为DHCP端口
    if (ntohs(udp_header->uh_sport) != 67 && ntohs(udp_header->uh_sport) != 68 &&
        ntohs(udp_header->uh_dport) != 67 && ntohs(udp_header->uh_dport) != 68) {
        printf("[警告] 检测到DHCP数据包，但UDP端口无效!   [%d]\n", ntohs(udp_header->uh_sport));
        return;
    }
    
    // 计算UDP数据包的长度
    int udp_payload_len = ntohs(udp_header->uh_ulen) - sizeof(struct udphdr);
    
    // 检查数据包长度是否足够容纳DHCP头部
    if (udp_payload_len < sizeof(struct dhcp_packet)) {
        // 数据包太短，不是DHCP包
        printf("[警告] 检测到DHCP数据包，但数据包太短，不是DHCP包!\n");
        return;
    }
    
    // 解析DHCP头部
    dhcp_pkt = (struct dhcp_packet *)(packet + sizeof(struct ether_header) + 
                                      (ip_header->ip_hl * 4) + sizeof(struct udphdr));
    
    // 验证魔数cookie
    if (ntohl(dhcp_pkt->magic_cookie) != DHCP_OPTIONS_MAGIC_COOKIE) {
        printf("[警告] 检测到DHCP数据包，但魔数cookie无效!   [%d]\n", ntohl(dhcp_pkt->magic_cookie));
        return; // 静默处理，不显示警告
    }
    
    // 检查是否为重放攻击
    if (is_dhcp_replay_attack(dhcp_pkt, current_time)) {
        char client_mac_str[18];
        sprintf(client_mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
                dhcp_pkt->chaddr[0], dhcp_pkt->chaddr[1], dhcp_pkt->chaddr[2],
                dhcp_pkt->chaddr[3], dhcp_pkt->chaddr[4], dhcp_pkt->chaddr[5]);
        
        printf("[警告] 检测到可能的DHCP重放攻击!\n");
        printf("       事务ID: 0x%lx\n", ntohl(dhcp_pkt->xid));
        printf("       客户端MAC: %s\n", client_mac_str);
        printf("       时间戳: %ld\n", current_time);
    }
    
    // 清理过期记录
    cleanup_dhcp_table(current_time);
}

// 数据包处理回调函数
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    struct ether_header *eth_header = (struct ether_header *)packet;
    
    if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        process_dhcp_packet(packet, pkthdr->len);
    }
}

int main() {
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;
    char filter_exp[] = "udp and (port 67 or port 68)";
    bpf_u_int32 net;
    
    // 查找默认设备
    char *dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
        fprintf(stderr, "找不到默认网络设备: %s\n", errbuf);
        return 1;
    }
    
    printf("设备: %s\n", dev);
    
    // 打开网络设备进行捕获
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "无法打开设备 %s: %s\n", dev, errbuf);
        return 1;
    }
    
    // 编译和设置过滤器
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
        fprintf(stderr, "无法解析过滤器 %s: %s\n", filter_exp, pcap_geterr(handle));
        return 1;
    }
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "无法安装过滤器 %s: %s\n", filter_exp, pcap_geterr(handle));
        return 1;
    }
    
    printf("开始DHCP重放攻击检测...\n");
    printf("监听DHCP包...\n");

    // 开始捕获数据包
    pcap_loop(handle, 0, packet_handler, NULL);
    
    // 关闭句柄
    pcap_close(handle);
    
    return 0;
}