#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#define TCP_TABLE_SIZE 4096
#define REPLAY_THRESHOLD 5      // 提高阈值以减少误报
#define TIME_WINDOW 20          // 时间窗口(秒)

// TCP连接信息结构
typedef struct {
    u_int32_t src_ip;        // 源IP地址
    u_int32_t dst_ip;        // 目标IP地址
    u_int16_t src_port;      // 源端口
    u_int16_t dst_port;      // 目标端口
    u_int32_t seq_num;       // 序列号
    time_t first_seen;       // 第一次见到该连接的时间
    time_t last_seen;        // 最后一次见到该连接的时间
    int count;               // 相同序列号出现的次数
} tcp_connection_info;

// 全局变量
tcp_connection_info tcp_table[TCP_TABLE_SIZE];
int tcp_table_size = 0;

// 清理过期的TCP连接记录
void cleanup_tcp_table(time_t current_time) {
    int i, j;
    for (i = 0; i < tcp_table_size; i++) {
        if (current_time - tcp_table[i].last_seen > 300) {
            for (j = i; j < tcp_table_size - 1; j++) {
                memcpy(&tcp_table[j], &tcp_table[j+1], sizeof(tcp_connection_info));
            }
            tcp_table_size--;
            i--;
        }
    }
}

// 检查是否为TCP重放攻击
int is_tcp_replay_attack(struct ip *ip_header, struct tcphdr *tcp_header, time_t current_time) {
    int i;
    int found = -1;
    
    u_int32_t src_ip = ip_header->ip_src.s_addr;
    u_int32_t dst_ip = ip_header->ip_dst.s_addr;
    u_int16_t src_port = ntohs(tcp_header->th_sport);
    u_int16_t dst_port = ntohs(tcp_header->th_dport);
    u_int32_t seq_num = ntohl(tcp_header->th_seq);
    
    printf("DEBUG: 检查重放攻击 - 源IP: %u.%u.%u.%u:%d, 目标IP: %u.%u.%u.%u:%d, 序列号: %u\n",
           src_ip & 0xFF, (src_ip >> 8) & 0xFF, (src_ip >> 16) & 0xFF, (src_ip >> 24) & 0xFF, src_port,
           dst_ip & 0xFF, (dst_ip >> 8) & 0xFF, (dst_ip >> 16) & 0xFF, (dst_ip >> 24) & 0xFF, dst_port,
           seq_num);
    
    for (i = 0; i < tcp_table_size; i++) {
        if (tcp_table[i].src_ip == src_ip && 
            tcp_table[i].dst_ip == dst_ip &&
            tcp_table[i].src_port == src_port &&
            tcp_table[i].dst_port == dst_port &&
            tcp_table[i].seq_num == seq_num) {
            found = i;
            printf("DEBUG: 找到匹配记录，索引: %d, 当前计数: %d\n", i, tcp_table[i].count);
            break;
        }
    }
    
    if (found != -1) {
        // 更新记录
        tcp_table[found].last_seen = current_time;
        tcp_table[found].count++;
        printf("DEBUG: 更新记录计数: %d\n", tcp_table[found].count);
        
        if (current_time - tcp_table[found].first_seen <= TIME_WINDOW) {
            printf("DEBUG: 在时间窗口内 (%ld 秒)\n", current_time - tcp_table[found].first_seen);
            if (tcp_table[found].count >= REPLAY_THRESHOLD) {
                // 发现重放攻击，重置计数器以便继续检测
                tcp_table[found].count = 1;
                tcp_table[found].first_seen = current_time;
                printf("DEBUG: 检测到重放攻击!\n");
                return 1;
            }
        } else {
            // 超出时间窗口，重置计数器和时间
            printf("DEBUG: 超出时间窗口，重置计数器\n");
            tcp_table[found].count = 1;
            tcp_table[found].first_seen = current_time;
        }
    } else {
        // 添加新记录
        printf("DEBUG: 未找到匹配记录，添加新记录\n");
        if (tcp_table_size < TCP_TABLE_SIZE) {
            tcp_table[tcp_table_size].src_ip = src_ip;
            tcp_table[tcp_table_size].dst_ip = dst_ip;
            tcp_table[tcp_table_size].src_port = src_port;
            tcp_table[tcp_table_size].dst_port = dst_port;
            tcp_table[tcp_table_size].seq_num = seq_num;
            tcp_table[tcp_table_size].first_seen = current_time;
            tcp_table[tcp_table_size].last_seen = current_time;
            tcp_table[tcp_table_size].count = 1;
            printf("DEBUG: 新记录添加完成，索引: %d\n", tcp_table_size);
            tcp_table_size++;
        } else {
            printf("DEBUG: TCP表已满，无法添加新记录\n");
        }
    }
    return 0;
}

// 处理TCP数据包
void process_tcp_packet(const u_char *packet, int packet_len) {
    struct ether_header *eth_header;
    struct ip *ip_header;
    struct tcphdr *tcp_header;
    time_t current_time = time(NULL);
    
    // 解析以太网头部
    eth_header = (struct ether_header *)packet;
    if (ntohs(eth_header->ether_type) != ETHERTYPE_IP) return;

    // 解析IP头部
    ip_header = (struct ip *)(packet + sizeof(struct ether_header));
    if (ip_header->ip_p != IPPROTO_TCP) return;

    // 解析TCP头部
    tcp_header = (struct tcphdr *)(packet + sizeof(struct ether_header) + (ip_header->ip_hl * 4));
    
    // // 检查TCP标志位，只关注SYN, ACK, PSH, FIN包
    u_int8_t tcp_flags = tcp_header->th_flags;
    if (!(tcp_flags & TH_SYN) ) {
        return;
    }

    // 检查是否为TCP重放攻击
    if (is_tcp_replay_attack(ip_header, tcp_header, current_time)) {
        char src_ip_str[16], dst_ip_str[16];
        sprintf(src_ip_str, "%u.%u.%u.%u", 
                ip_header->ip_src.s_addr & 0xFF,
                (ip_header->ip_src.s_addr >> 8) & 0xFF,
                (ip_header->ip_src.s_addr >> 16) & 0xFF,
                (ip_header->ip_src.s_addr >> 24) & 0xFF);
                
        sprintf(dst_ip_str, "%u.%u.%u.%u",
                ip_header->ip_dst.s_addr & 0xFF,
                (ip_header->ip_dst.s_addr >> 8) & 0xFF,
                (ip_header->ip_dst.s_addr >> 16) & 0xFF,
                (ip_header->ip_dst.s_addr >> 24) & 0xFF);
        
        printf("[警告] 检测到可能的TCP重放攻击!\n");
        printf("       源IP: %s:%d\n", src_ip_str, ntohs(tcp_header->th_sport));
        printf("       目标IP: %s:%d\n", dst_ip_str, ntohs(tcp_header->th_dport));
        printf("       序列号: %u\n", ntohl(tcp_header->th_seq));
        printf("       TCP标志: %s%s%s%s%s%s\n",
               (tcp_header->th_flags & TH_SYN) ? "SYN " : "",
               (tcp_header->th_flags & TH_ACK) ? "ACK " : "",
               (tcp_header->th_flags & TH_PUSH) ? "PSH " : "",
               (tcp_header->th_flags & TH_FIN) ? "FIN " : "",
               (tcp_header->th_flags & TH_RST) ? "RST " : "",
               (tcp_header->th_flags & TH_URG) ? "URG " : "");
        printf("       时间戳: %ld\n", current_time);
    }
    
    // 清理过期记录
    cleanup_tcp_table(current_time);
}

// 数据包处理回调函数
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    struct ether_header *eth_header = (struct ether_header *)packet;
    
    if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        process_tcp_packet(packet, pkthdr->len);
    }
}

int main() {
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;
    char filter_exp[] = "tcp";
    bpf_u_int32 net;
    
    // 查找默认设备
    char *dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
        fprintf(stderr, "找不到默认网络设备: %s\n", errbuf);
        return 1;
    }
    

    // 打开网络设备进行捕获
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "无法打开设备 %s: %s\n", dev, errbuf);
        return 1;
    }
    printf("打开的网口 = %s\n", dev);
    // 编译和设置过滤器
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
        fprintf(stderr, "无法解析过滤器 %s: %s\n", filter_exp, pcap_geterr(handle));
        return 1;
    }
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "无法安装过滤器 %s: %s\n", filter_exp, pcap_geterr(handle));
        return 1;
    }
    
    printf("开始TCP重放攻击检测...\n");
    printf("监听TCP包... (阈值: %d, 时间窗口: %d秒)\n", REPLAY_THRESHOLD, TIME_WINDOW);

    // 开始捕获数据包
    pcap_loop(handle, 0, packet_handler, NULL);
    
    // 关闭句柄
    pcap_close(handle);
    
    return 0;
}