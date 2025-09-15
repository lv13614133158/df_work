#include <pcap.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

/**
 * 数据包处理函数，用于解析网络数据包并提取SSL/TLS版本信息
 * @param user_data 用户自定义数据指针，本函数中未使用
 * @param pkthdr 指向数据包头部信息的指针，包含时间戳和长度等信息
 * @param packet 指向实际数据包内容的指针
 */
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    // 解析以太网帧头部，获取帧类型信息
    struct ether_header *eth_header = (struct ether_header*)packet;
    
    // 判断是否为IP协议类型的数据包
    if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        // 解析IP头部，获取IP协议版本和头部长度
        struct ip *ip_header = (struct ip*)(packet + sizeof(struct ether_header));
        
     
        
 
        if (ip_header->ip_p == IPPROTO_TCP) {
            // 解析TCP头部，获取源端口和目标端口
            struct tcphdr *tcp_header = (struct tcphdr*)(packet + sizeof(struct ether_header) + ip_header->ip_hl*4);
            
            // 检查TCP端口是否为SSL/TLS常用的443端口
            if (ntohs(tcp_header->th_dport) == 443 || ntohs(tcp_header->th_sport) == 443) {
                // 计算TCP数据载荷在数据包中的偏移位置
                int data_offset = sizeof(struct ether_header) + ip_header->ip_hl*4 + tcp_header->th_off*4;
                int data_len = pkthdr->len - data_offset;
                
             
            if (data_len >= 3) {
                const u_char *payload = packet + data_offset;
                

                            if (payload[0] == 0x16) {
                                u_char major_version = payload[1];
                                u_char minor_version = payload[2];
                                printf("IP Version: %d\n", ip_header->ip_v);
                            
                                printf("TCP Source : %s\n", inet_ntoa(ip_header->ip_src));
                                printf("TCP Source Port: %d\n", ntohs(tcp_header->th_sport));
                                printf("TCP Destination : %s\n", inet_ntoa(ip_header->ip_dst));
                                printf("TCP Destination Port: %d\n", ntohs(tcp_header->th_dport));
                                switch (major_version) {
                                    case 3:
                                        switch (minor_version) {
                                            case 0:
                                                printf("SSL/TLS Version: SSL 3.0\n");
                                                break;
                                            case 1:
                                                printf("SSL/TLS Version: TLS 1.0\n");
                                                break;
                                            case 2:
                                                printf("SSL/TLS Version: TLS 1.1\n");
                                                break;
                                            case 3:
                                                printf("SSL/TLS Version: TLS 1.2\n");
                                                break;
                                            case 4:
                                                printf("SSL/TLS Version: TLS 1.3\n");
                                                break;
                                            default:
                                                printf("SSL/TLS Version: %d.%d (Unknown TLS version)\n", major_version, minor_version);
                                        }
                                        break;
                                    default:
                                        printf("SSL/TLS Version: %d.%d (Unknown SSL/TLS version)\n", major_version, minor_version);
                                }
                                 printf("\n");
                            } else {
                            
                            }
                           
                        }
                    }

                }
            }
}

int main() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    
    // 查找默认设备
    char *dev = pcap_lookupdev(errbuf);
    if (dev == NULL) {
        fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
        return 1;
    }
    
    printf("Device: %s\n", dev);
    
    // 打开设备进行捕获
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        return 2;
    }
    
    // 开始捕获数据包
    pcap_loop(handle, 0, packet_handler, NULL);
    
    // 关闭设备
    pcap_close(handle);
    return 0;
}