#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "idsFrame.h"
#include "can_parser.h"

#define CAN_PARSER_MAX_INPUT (0x4000)     // input limit 16k
#define CAN_PARSER_BUFF_LENS (0x8000)     // twice input limit 32k
#define CAN_MIN_TRANSFER_LAYER (8)        // head[4] + tail[4]

typedef struct
{
    char *data_buff;
    unsigned int data_lens;
} CAN_PARSER_HANDLE_T;

static CAN_PARSER_HANDLE_T* s_can_parser_handle = NULL;

static unsigned short calcCRC16(const char *data, unsigned short dataSize)
{
    if ((NULL == data) || (0 == dataSize))
    {
        return 0;
    }

    unsigned short i = 0;
    unsigned short j = 0;
    unsigned short r = 0xFFFFU;

    for (i = 0; i < dataSize; ++i)
    {
        r ^= (unsigned short)(data[i] << (16 - 8));
        for (j = 0; j < 8; ++j)
        {
            if ((r & 0x8000U) != 0)
            {
                r = (unsigned short)((r << 1) ^ 0x1021U);
            }
            else
            {
                r <<= 1;
            }
        }
    }

    return (unsigned short)(r & 0xFFFFU);
}

int can_read_u32(unsigned      char *puData, unsigned int *puNum)
{
	unsigned char *puDst = (unsigned char *)puNum;
	puDst[0] = puData[3];
	puDst[1] = puData[2];
	puDst[2] = puData[1];
	puDst[3] = puData[0];

	return 0;
}

int can_read_u64(unsigned      char *puData, unsigned long long *puNum)
{
	unsigned char *puDst = (unsigned char *)puNum;
	puDst[0] = puData[7];
	puDst[1] = puData[6];
	puDst[2] = puData[5];
	puDst[3] = puData[4];
	puDst[4] = puData[3];
	puDst[5] = puData[2];
	puDst[6] = puData[1];
	puDst[7] = puData[0];

	return 0;
}

int can_parser_handdle_release(CAN_PARSER_HANDLE_T** handle)
{
    if (handle == NULL || *handle == NULL)
    {
        return -1;
    }

    free(*handle);
    *handle = NULL;

    return 0;
}


int can_parser_handle_create(CAN_PARSER_HANDLE_T** handle)
{
    if (handle == NULL)
    {
        return -1;
    }

    if (*handle != NULL)
    {
        can_parser_handdle_release(handle);
    }

    *handle = (CAN_PARSER_HANDLE_T*)malloc(sizeof(CAN_PARSER_HANDLE_T) + CAN_PARSER_BUFF_LENS);
    if (*handle == NULL)
    {
        return -1;
    }
    memset(*handle, 0, sizeof(CAN_PARSER_HANDLE_T) + CAN_PARSER_BUFF_LENS);

    (*handle)->data_buff = (char*)(*handle) + sizeof(CAN_PARSER_HANDLE_T);
    (*handle)->data_lens = 0;

    return 0;
}

static int can_parser_business(unsigned char *in, unsigned int in_lens)
{
    // 解析每条数据信息
    int numData = 0;
    int offset = 0;
    unsigned char* data = in;
    CAN_DATA_INFO_T_A output = {0};

    //printf("[zy_test] in_lens:%d\n", in_lens);

    while (offset < in_lens)
    {
        // 检查剩余数据长度是否足够
        if (offset + 17 > in_lens) {
            return numData;
        }
        
        // 解析时间戳
        //output.time_stamps = (unsigned int)((data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3]);
        memcpy(output.time_stamps, &data[offset], 10);
		/*printf("%x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
			data[offset],data[offset+1],data[offset+2],data[offset+3],data[offset+4],data[offset+5],data[offset+6],data[offset+7],
			data[offset+8],data[offset+9]);*/

		unsigned char data_ull[8] = {0};
		data_ull[2] = data[offset + 0];
		data_ull[3] = data[offset + 1];
		data_ull[4] = data[offset + 2];
		data_ull[5] = data[offset + 3];
		data_ull[6] = data[offset + 4];
		data_ull[7] = data[offset + 5];
		unsigned long long puNum_ull = 0;
		can_read_u64(data_ull, &puNum_ull);
		//printf("puNum_ull:%llu\n", puNum_ull);

		unsigned int puNum_ui = 0;
		can_read_u32(data + offset + 6, &puNum_ui);
		//printf("puNum_ui:%u\n", puNum_ui);
		//printf("time_stamps:%llu.%u S\n", puNum_ull, puNum_ui);

        // 解析CAN ID
        output.canid = (unsigned int)((data[offset + 10] << 24) | (data[offset + 11] << 16) | (data[offset + 12] << 8) | data[offset + 13]);

        // 解析方向、通道、payload长度
        output.direction = data[offset + 14];
        output.channel = data[offset + 15];
        output.payload_length = data[offset + 16];

        // 解析payload
        for (int i = 0; i < output.payload_length; i++)
        {
            output.payload[i] = data[offset + 17 + i];
        }

#if 0
        printf("time:%f, id:0x%x, dir:%d, channel:%d, len:%d,",
			(puNum_ull + (puNum_ui / 1000000000.0)), output.canid, output.direction, output.channel, output.payload_length);

        for (int i = 0; i < output.payload_length; i++)
        {
			printf(" %X", output.payload[i]);
        }
        printf("\n");
#endif

		//float time1 = (puNum_ull + (puNum_ui / 1000000000.0));
		double time = (puNum_ull + (puNum_ui / 1000000000.0));
		//printf("time1:%f, time:%lf\n", time1, time);
        can_device_pub_dat(output.channel, output.canid, output.payload, output.payload_length, time);
        numData++;
        offset += 17 + output.payload_length;
    }

    return numData;
}

unsigned int can_parser_decode(CAN_PARSER_HANDLE_T* handle, unsigned char* in, unsigned int in_lens)
{
    unsigned char* head = NULL;
    unsigned char* cur = NULL;
    unsigned char* tail = NULL;
    unsigned char* transfer_head = NULL;
    unsigned char decode_result = -1;
    unsigned short data_length = 0;
    unsigned short crc = 0;

    if (in_lens > CAN_PARSER_MAX_INPUT)
    {
        printf("input is too much\n");
        return -1;
    }

    if (handle == NULL || in == NULL)
    {
        printf("input point is null\n");
        return -1;
    }

    /*
     * If transfer store buff is filled with some data, we use transfer
     * store buff as current working data buff, or we use input data buff
     * as current working data buff.
     * This method could adapt to the most kinds of input io type with less cost.
     */
    if (handle->data_lens > 0)
    {
        if (handle->data_lens + in_lens > CAN_PARSER_BUFF_LENS)
        {
            printf("buff overflow\n");
            handle->data_lens = 0;
            head = in;
            tail = head + in_lens;
        }
        else
        {
            memcpy(handle->data_buff + handle->data_lens, in, in_lens);
            head = handle->data_buff;
            tail = head + (handle->data_lens + in_lens);
        }
    }
    else
    {
        head = in;
        tail = head + in_lens;
    }

    cur = head;
    while (cur + 1 < tail)
    {
        if (*cur == 0xFF && *(cur + 1) == 0xFD)
        {
            transfer_head = cur;
            if ((tail - transfer_head) < CAN_MIN_TRANSFER_LAYER)
            {
                break;
            }

            data_length = (cur[2] << 8) | cur[3];
            if ((tail - transfer_head) < CAN_MIN_TRANSFER_LAYER + data_length)
            {
                break;
            }

            // check transfer layer tail(key word)
            if (*(transfer_head + 6 + data_length) != 0xFF || *(transfer_head + 7 + data_length) != 0xFE)
            {
                cur++;
                continue;
            }
            crc = (transfer_head[4 + data_length] << 8) | transfer_head[5 + data_length];
            //printf("crc:%d\n", crc);
			unsigned short data_crc = calcCRC16(transfer_head + 4, data_length);
			//printf("data_crc:%d\n", data_crc);
			if (crc == data_crc)
            {
                decode_result = can_parser_business(transfer_head + 4, data_length);
            }
            else
            {
                printf("crc check fail!\n");
            }

            transfer_head = NULL;
            cur += (CAN_MIN_TRANSFER_LAYER + data_length);
        }
        else
        {
            cur++;
        }
    }

    // store the transfer layer data
    if (transfer_head == NULL)
    {
        if (cur >= tail)
        {
            handle->data_lens = 0;
        }
        else
        {
            if (*cur == 0xFF)
            {
                handle->data_buff[0] = 0xFF;
                handle->data_lens = 1;
            }
            else
            {
                handle->data_lens = 0;
            }
        }
    }
    else
    {
        if (handle->data_lens == 0)
        {
            memcpy(handle->data_buff, transfer_head, tail - transfer_head);
        }
        else
        {
            memmove(handle->data_buff, transfer_head, tail - transfer_head);
        }

        handle->data_lens = tail - transfer_head;
    }

    return decode_result;
}

int can_parse_data(unsigned char* data, size_t length, CAN_DATA_INFO_T *output)
{
	// can parser handle init
    if (s_can_parser_handle == NULL)
    {
        can_parser_handle_create(&s_can_parser_handle);
        if (s_can_parser_handle == NULL)
        {
            return -1;
        }
    }

    return can_parser_decode(s_can_parser_handle, data, length);
}
