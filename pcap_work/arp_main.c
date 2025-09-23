#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>

#define ARP_TABLE_SIZE 256
#define TIME_WINDOW 10

// 存储ARP映射和统计信息的结构
typedef struct {
    u_char mac[6];              // MAC地址
    u_char ip[4];               // IP地址
    time_t last_seen;           // 最后一次见到该ARP记录的时间
    time_t last_reply_time;     // 上一次收到ARP回复的时间
    int duplicate_count;        // 连续重复ARP回复计数
} arp_host_info;

// 全局变量存储ARP信息
arp_host_info arp_table[ARP_TABLE_SIZE];
int arp_table_size = 0;

// ARP头部结构
struct arp_header {
    u_short hardware_type;
    u_short protocol_type;
    u_char hardware_len;
    u_char protocol_len;
    u_short opcode;
    u_char sender_mac[6];
    u_char sender_ip[4];
    u_char target_mac[6];
    u_char target_ip[4];
};

// 将MAC地址转换为字符串
void mac_to_str(u_char *mac, char *str) {
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

// 将IP地址转换为字符串
void ip_to_str(u_char *ip, char *str) {
    sprintf(str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
}

// 清理过期的ARP记录
void cleanup_arp_table(time_t current_time) {
    int i, j;
    for (i = 0; i < arp_table_size; i++) {
        // 清理长时间未见的记录（例如超过5分钟）
        if (current_time - arp_table[i].last_seen > 300) {
            for (j = i; j < arp_table_size - 1; j++) {
                memcpy(&arp_table[j], &arp_table[j+1], sizeof(arp_host_info));
            }
            arp_table_size--;
            i--;
        }
    }
}

// 回调函数，处理每个捕获的数据包
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    struct ether_header *eth_header;
    struct arp_header *arp_hdr;
    int i;
    time_t current_time = time(NULL);
    
    // 清理过期记录
    cleanup_arp_table(current_time);

    // 跳过以太网头部
    eth_header = (struct ether_header *)packet;
    
    // 只处理ARP包
    if (ntohs(eth_header->ether_type) == ETHERTYPE_ARP) {
        // 定位ARP头部
        arp_hdr = (struct arp_header *)(packet + sizeof(struct ether_header));
        
        // 只处理ARP应答
        if (ntohs(arp_hdr->opcode) != 2) {
            // printf("收到ARP非应答包 - 忽略\n");
            return;
        }
        
        // 显示发送者信息
        char sender_ip_str[16];
        char sender_mac_str[18];
        ip_to_str(arp_hdr->sender_ip, sender_ip_str);
        mac_to_str(arp_hdr->sender_mac, sender_mac_str);
        
        printf("收到ARP应答包 - 发送者IP: %s, MAC: %s\n", sender_ip_str, sender_mac_str);
        
        // 查找是否已存在该MAC地址的记录
        int mac_exists = -1;
        for (i = 0; i < arp_table_size; i++) {
            if (memcmp(arp_table[i].mac, arp_hdr->sender_mac, 6) == 0) {
                mac_exists = i;
                break;
            }
        }
        
        // 如果已存在该MAC地址的记录
        if (mac_exists != -1) {
            printf("找到已存在的记录\n");
            
            // 检查时间间隔并检查重复次数
            if (current_time - arp_table[mac_exists].last_reply_time < 1) {
                // 时间间隔小于1秒，增加重复计数
                arp_table[mac_exists].duplicate_count++;
                printf("时间间隔过短，重复计数: %d\n", arp_table[mac_exists].duplicate_count);
                
                // 如果重复次数超过阈值(1次)，认为是ARP重放攻击
                if (arp_table[mac_exists].duplicate_count > 1) {
                    printf("[警告] 检测到ARP重放攻击!\n");
                    printf("       IP: %s, MAC: %s\n", sender_ip_str, sender_mac_str);
                    printf("       连续重复回复次数: %d\n", arp_table[mac_exists].duplicate_count);
                }
            } else {
                // 时间间隔较长，重置重复计数
                printf("时间间隔正常，重置重复计数\n");
                arp_table[mac_exists].duplicate_count = 0;
            }
            
            // 更新时间戳
            arp_table[mac_exists].last_seen = current_time;
            arp_table[mac_exists].last_reply_time = current_time;
            
            // 更新IP地址（如果发生变化）
            if (memcmp(arp_table[mac_exists].ip, arp_hdr->sender_ip, 4) != 0) {
                printf("IP地址发生变化，更新IP地址\n");
                memcpy(arp_table[mac_exists].ip, arp_hdr->sender_ip, 4);
            }
        } else {
            // 添加新记录
            if (arp_table_size < ARP_TABLE_SIZE) {
                printf("添加新的ARP记录\n");
                memcpy(arp_table[arp_table_size].mac, arp_hdr->sender_mac, 6);
                memcpy(arp_table[arp_table_size].ip, arp_hdr->sender_ip, 4);
                arp_table[arp_table_size].last_seen = current_time;
                arp_table[arp_table_size].last_reply_time = current_time;
                arp_table[arp_table_size].duplicate_count = 0;
                arp_table_size++;
            }
        }
        printf("-------------------\n");
    }
}

int main() {
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;
    char filter_exp[] = "arp";
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
    
    printf("开始ARP重放攻击检测...\n");
    printf("监听ARP应答包...\n");

    // 开始捕获数据包
    pcap_loop(handle, 0, packet_handler, NULL);
    
    // 关闭句柄
    pcap_close(handle);
    
    return 0;
}