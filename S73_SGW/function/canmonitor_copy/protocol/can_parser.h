#ifndef __CAN_PARSER_H_
#define __CAN_PARSER_H_

#define CAN_BUFFER_SIZE 1024

typedef struct
{
    unsigned int time_stamps;
    unsigned int canid;
    unsigned char direction;
    unsigned char channel;
    unsigned char payload_length;
    unsigned char payload[CAN_BUFFER_SIZE];
}CAN_DATA_INFO_T;

typedef struct
{
    unsigned char time_stamps[10];
    unsigned int canid;
    unsigned char direction;
    unsigned char channel;
    unsigned char payload_length;
    unsigned char payload[CAN_BUFFER_SIZE];
}CAN_DATA_INFO_T_A;

int can_parse_data(unsigned char* data, size_t length, CAN_DATA_INFO_T *output);

#endif
