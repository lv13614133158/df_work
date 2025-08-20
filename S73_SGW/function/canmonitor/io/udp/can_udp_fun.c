#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include "can_parser.h"
#include "can_udp_fun.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6690
#define CLIENT_PORT 6691

int can_udp_server()
{
    int sockfd;
    struct sockaddr_in server_addr;

	printf("udp_server init!\n");
    // 创建socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed\n");
        return -1;
    }

    // 设置服务器地址信息
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(CLIENT_PORT);
    /*if (inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr)) <= 0) {
        printf("inet_pton failed\n");
        exit(EXIT_FAILURE);
    }*/
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
    {
        printf("bind failed\n");
		return -1;
    }


	CAN_DATA_INFO_T outputData[128];

	while (1)
	{
		char buf[10240] = {0};
		int len = 0;

		if ((len = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, NULL)) < 0)
		{
			printf("recvfrom err!\n");
            sleep(1);
		}
		//printf("recv len:%d\n", len);

        if (len  > 0)
        {
            int numData = can_parse_data(buf, len, outputData);
        }
    }

    close(sockfd);
    return 0;
}

