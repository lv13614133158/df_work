#include "ql_oe.h"
#include "get_imei.h"

#define QUEC_AT_PORT    "/dev/smd8"

static int smd_fd = -1;
static int Ql_SendAT(char* atCmd, char* finalRsp, long timeout_ms,char *buf);
int get_imei(char *buf, int len);

int get_device_sn(char *buf,int len)
{
	int ret = -1;
    printf("get_device_sn .....\n");
	ret = get_imei(buf,len);
	return ret;	
}

int get_imei(char *buf, int len)
{
   FILE *fp=NULL; 
   char buff[128]={0};   
   int smd_fd = open("/etc/config/system.ini", O_RDWR | O_NONBLOCK | O_NOCTTY);
   if(smd_fd <= 0){
		return 1;
   }
   close(smd_fd);
   memset(buff,0,sizeof(buff)); 
   fp=popen("tail /etc/config/system.ini -n 1","r"); 
   fread(buff,1,127,fp);//将fp的数据流读到buff中   
   sscanf(buff,"serial=%s",buf);
   pclose(fp);   
   return 0;
}

static int Ql_SendAT(char* atCmd, char* finalRsp, long timeout_ms,char *buf)
{
    int iRet;
    int iLen;
    fd_set fds;
    int rdLen;
#define lenToRead 100
    char strAT[100];
    char strFinalRsp[100];
    char strResponse[100];
    struct timeval timeout = {0, 0};
    boolean bRcvFinalRsp = FALSE;

    memset(strAT, 0x0, sizeof(strAT));
    iLen = sizeof(atCmd);
    strncpy(strAT, atCmd, iLen);

    sprintf(strFinalRsp, "\r\n%s", finalRsp);
    
	timeout.tv_sec  = timeout_ms / 1000;
	timeout.tv_usec = timeout_ms % 1000;
    
    
    // Add <cr><lf> if needed
    iLen = strlen(atCmd);
    if ((atCmd[iLen-1] != '\r') && (atCmd[iLen-1] != '\n'))
    {
        iLen = sprintf(strAT, "%s\r\n", atCmd); 
        strAT[iLen] = 0;
    }

    // Send AT
    iRet = write(smd_fd, strAT, iLen);
    // printf(">>Send AT: \"%s\", iRet=%d\n", atCmd, iRet);

	while (1)
	{
		FD_ZERO(&fds); 
		FD_SET(smd_fd, &fds); 

		switch (select(smd_fd + 1, &fds, NULL, NULL, &timeout))
		//switch (select(smd_fd + 1, &fds, NULL, NULL, NULL))	// block mode
		{
		case -1: 
			printf("< select error >\n");
			return -1;

		case 0:
			printf("< time out >\n");
			return 1; 

		default: 
			if (FD_ISSET(smd_fd, &fds)) 
			{
				do {
					memset(strResponse, 0x0, sizeof(strResponse));
					rdLen = read(smd_fd, strResponse, lenToRead);
					// printf(">>Read response/urc, len=%d, content:\n%s\n", rdLen, strResponse);
					//  printf("rcv:%s\n", strResponse);
					// printf("final rsp:%s\n", strFinalRsp);
					sscanf(strResponse,"%*[^:]:%s",buf);
				
					if ((rdLen > 0) && strstr(strResponse, strFinalRsp))
					{
					    if (strstr(strResponse, strFinalRsp)     // final OK response
					       || strstr(strResponse, "+CME ERROR:") // +CME ERROR
					       || strstr(strResponse, "+CMS ERROR:") // +CMS ERROR
					       || strstr(strResponse, "ERROR"))      // Unknown ERROR
					    {
					        bRcvFinalRsp = TRUE;
					    }else{
					        printf("\n< not final rsp >\n");
					    }
					}
				} while ((rdLen > 0) && (lenToRead == rdLen));
			}else{
				printf("FD is missed\n");
			}
			break;
		}

		// Found the final response , return back
		if (bRcvFinalRsp)
		{
		    break;
		}	
   	}
   	return 0;
}

