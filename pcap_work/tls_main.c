#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

/**
 * 在Server Hello消息中检测TLS 1.3
 * @param server_hello_data Server Hello消息数据指针
 * @param server_hello_len Server Hello消息长度
 * @return 1表示检测到TLS 1.3，0表示未检测到
 */
int detect_tls13_in_server_hello(const u_char *server_hello_data, int server_hello_len) {
    // 确保有足够的数据进行解析
    if (server_hello_len < 38) {
        return 0;
    }
    
    // Server Hello结构解析
    // 跳过Handshake头部 (type: 1 byte + length: 3 bytes)
    // 跳过Version (2 bytes) + Random (32 bytes)
    const u_char *ptr = server_hello_data + 38;
    int offset = 38;
    
    // 跳过Session ID
    if (offset + 1 > server_hello_len) {
        return 0;
    }
    int session_id_len = ptr[0];
    if (offset + 1 + session_id_len > server_hello_len) {
        return 0;
    }
    offset += 1 + session_id_len;
    ptr += 1 + session_id_len;
    
    // 跳过Cipher Suite (2 bytes)
    if (offset + 2 > server_hello_len) {
        return 0;
    }
    offset += 2;
    ptr += 2;
    
    // 跳过Compression Method (1 byte)
    if (offset + 1 > server_hello_len) {
        return 0;
    }
    offset += 1;
    ptr += 1;
    
    // 检查Extensions是否存在
    if (offset + 2 > server_hello_len) {
        return 0;
    }
    int extensions_len = (ptr[0] << 8) | ptr[1];
    if (offset + 2 + extensions_len > server_hello_len) {
        return 0;
    }
    offset += 2;
    ptr += 2;
    
    // 遍历Extensions
    int extensions_end = offset + extensions_len;
    while (offset < extensions_end && offset + 4 <= server_hello_len) {
        if (offset + 4 > server_hello_len) {
            break;
        }
        
        int extension_type = (ptr[0] << 8) | ptr[1];
        int extension_length = (ptr[2] << 8) | ptr[3];
        
        // 检查是否为supported_versions扩展 (类型0x002b)
        if (extension_type == 0x002b) {
            // Server Hello中的supported_versions扩展包含协商后的版本
            if (extension_length >= 2 && offset + 4 + 2 <= server_hello_len) {
                u_char major = ptr[4];
                u_char minor = ptr[5];
                if (major == 0x03 && minor == 0x04) {
                    return 1; // TLS 1.3
                }
            }
        }
        
        // 移动到下一个扩展
        offset += 4 + extension_length;
        if (offset <= server_hello_len && extension_length <= server_hello_len) {
            ptr += 4 + extension_length;
        } else {
            break;
        }
    }
    
    return 0; // 不是TLS 1.3
}

/**
 * 数据包处理函数，只处理Server Hello消息
 * @param user_data 用户自定义数据指针
 * @param pkthdr 指向数据包头部信息的指针
 * @param packet 指向实际数据包内容的指针
 */
void packet_handler(u_char *user_data, const struct pcap_pkthdr *pkthdr, const u_char *packet) {
    // 解析以太网帧头部
    struct ether_header *eth_header = (struct ether_header*)packet;
    
    // 判断是否为IP协议类型的数据包
    if (ntohs(eth_header->ether_type) == ETHERTYPE_IP) {
        // 解析IP头部
        struct ip *ip_header = (struct ip*)(packet + sizeof(struct ether_header));
        
        if (ip_header->ip_p == IPPROTO_TCP) {
            // 解析TCP头部
            struct tcphdr *tcp_header = (struct tcphdr*)(packet + sizeof(struct ether_header) + ip_header->ip_hl*4);
            
            // 检查TCP端口是否为SSL/TLS常用的443端口
            if (ntohs(tcp_header->th_dport) == 443 || ntohs(tcp_header->th_sport) == 443) {
                // 计算TCP数据载荷在数据包中的偏移位置
                int data_offset = sizeof(struct ether_header) + ip_header->ip_hl*4 + tcp_header->th_off*4;
                int data_len = pkthdr->len - data_offset;
                
                if (data_len >= 6) {
                    const u_char *payload = packet + data_offset;

                    // 检查是否为TLS Handshake协议
                    if (payload[0] == 0x16) {
                        u_char handshake_type = payload[5];
                        
                        // 只处理Server Hello消息 (类型为0x02)
                        if (handshake_type == 0x02) {
                            uint16_t record_length = (payload[3] << 8) | payload[4];
                            
                            printf("Source: %s:%d\n", inet_ntoa(ip_header->ip_src), ntohs(tcp_header->th_sport));
                            printf("Destination: %s:%d\n", inet_ntoa(ip_header->ip_dst), ntohs(tcp_header->th_dport));
                            
                            // 检查数据长度是否有效
                            if (record_length >= 38 && record_length <= (data_len - 5)) {
                                int is_tls13 = detect_tls13_in_server_hello(payload + 5, record_length);
                                printf("SSL/TLS Version: %s (Server Hello)\n", is_tls13 ? "TLS 1.3" : "TLS 1.2 or earlier");
                            } else {
                                printf("SSL/TLS Version: Unknown (invalid Server Hello length)\n");
                            }
                            printf("\n");
                        }
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