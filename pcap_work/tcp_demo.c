#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

// 计算校验和
unsigned short checksum(unsigned short *ptr, int nbytes) {
    register long sum = 0;
    unsigned short oddbyte;
    register short answer;

    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((u_char*)&oddbyte) = *(u_char*)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short)~sum;
    
    return(answer);
}

int main() {
    int sockfd;
    char packet[4096];
    struct iphdr *iph = (struct iphdr *) packet;
    struct tcphdr *tcph = (struct tcphdr *) (packet + sizeof(struct iphdr));
    struct sockaddr_in sin;
    int one = 1;
    const int *val = &one;
    
    // 固定的序列号
    unsigned int fixed_sequence = 987654321;
    
    printf("发送具有固定序列号(%u)的TCP SYN包...\n", fixed_sequence);
    
    // 创建原始套接字
    sockfd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("创建原始套接字失败，请使用sudo运行");
        return -1;
    }
    
    // 设置套接字选项
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
        perror("设置套接字选项失败");
        close(sockfd);
        return -1;
    }
    
    // 清零包内容
    memset(packet, 0, 4096);
    
    // 设置目标地址
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr("192.168.196.128");
    
    // 填充IP头部
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
    iph->id = htonl(54321);
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;
    iph->saddr = inet_addr("192.168.196.1");
    iph->daddr = sin.sin_addr.s_addr;
    
    // 填充TCP头部（使用固定序列号）
    tcph->source = htons(12345);
    tcph->dest = htons(80);
    tcph->seq = htonl(fixed_sequence);  // 固定序列号
    tcph->ack_seq = 0;
    tcph->doff = 5;
    tcph->fin = 0;
    tcph->syn = 1;  // SYN标志
    tcph->rst = 0;
    tcph->psh = 0;
    tcph->ack = 0;
    tcph->urg = 0;
    tcph->window = htons(5840);
    tcph->check = 0;
    tcph->urg_ptr = 0;
    
    // 发送多个具有相同序列号的包
    for (int i = 0; i < 15; i++) {
        // 每次发送前改变源端口以避免被系统过滤
        tcph->source = htons(10000 );
        
        // 重新计算IP校验和
        iph->check = 0;
        iph->check = checksum((unsigned short *) packet, iph->tot_len >> 1);
        
        // 发送包
        if (sendto(sockfd, packet, iph->tot_len, 0, 
                   (struct sockaddr *)&sin, sizeof(sin)) < 0) {
            perror("发送包失败");
        } else {
            printf("发送包 %d，序列号: %u\n", i+1, fixed_sequence);
        }
        
        usleep(100);  // 100ms间隔
    }
    
    close(sockfd);
    printf("发送完成\n");
    return 0;
}