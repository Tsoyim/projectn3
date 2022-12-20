
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <math.h>
#include <sys/time.h>

#define MAX_COMMAND_LENGTH 16
#define AUTOMATED_FILENAME 512
typedef unsigned char uchar_t;
#define SWS 8//滑动窗口大小
#define RWS 8//滑动窗口大小
#define MAX_LEN 16//最大窗口长度
#define MAX_FRAME_SIZE 32

//TODO: You should change this!
//Remember, your frame can be AT MOST 32 bytes!
#define FRAME_PAYLOAD_SIZE 20
//System configuration information

struct Frame_t
{
    uint16_t src_id;//源ID
    uint16_t dst_id;//目的ID
    uint16_t is_end;//帧结束标识符
    uint16_t length;//数据长度
    uint8_t id;//帧ID（序号）
    uint8_t ack;//ACK序号
    char data[FRAME_PAYLOAD_SIZE];
    uint16_t CRC;//CRC16校验码
};
typedef struct Frame_t Frame;

struct SysConfig_t
{
    float drop_prob;//丢包率
    float corrupt_prob;//比特错误率
    unsigned char automated;
    char automated_file[AUTOMATED_FILENAME];
};
typedef struct SysConfig_t  SysConfig;

//Command line input information
struct Cmd_t
{
    uint16_t src_id;
    uint16_t dst_id;
    char * message;
};
typedef struct Cmd_t Cmd;

//Linked list information
enum LLtype 
{
    llt_string,
    llt_frame,
    llt_integer,
    llt_head
} LLtype;

struct LLnode_t
{
    struct LLnode_t * prev;
    struct LLnode_t * next;
    enum LLtype type;

    void * value;
};
typedef struct LLnode_t LLnode;


//Receiver and sender data structures
struct Receiver_Windows_t{
    Frame * frame;
    int received;
};
typedef struct Receiver_Windows_t Receiver_Windows;
struct Receiver_t
{
    //DO NOT CHANGE:
    // 1) buffer_mutex
    // 2) buffer_cv
    // 3) input_framelist_head
    // 4) recv_id
    pthread_mutex_t buffer_mutex;
    pthread_cond_t buffer_cv;
    LLnode * input_framelist_head;
    int recv_id;


    //补充的变量
    uint8_t NFE;//期待收到的帧序号
    uint8_t lastACK;//上一个帧结束的ACK的id
    // Frame buffer[MAX_LEN];//缓冲区
    Receiver_Windows r_Win[SWS];

};



struct Sender_Windows_t{
    Frame *frame;
    struct timeval* time_val;//发送时间
    u_int8_t is_ack;//是否接收了ACK,是的话为1,否则为0
};

typedef struct Sender_Windows_t Sender_Windows;

struct Sender_t
{
    //DO NOT CHANGE:
    // 1) buffer_mutex
    // 2) buffer_cv
    // 3) input_cmdlist_head
    // 4) input_framelist_head
    // 5) send_id
    pthread_mutex_t buffer_mutex;
    pthread_cond_t buffer_cv;    
    LLnode * input_cmdlist_head;
    LLnode * input_framelist_head;
    int send_id;
    //补充的变量
    LLnode * wait_cmdlist_head;//等待队列
    u_int8_t LAR;//窗口LAR,逻辑下界
    u_int8_t LFS;//窗口LFS,逻辑上界
    Sender_Windows s_Win[SWS];//维护发送窗口
    // Frame buffer[MAX_LEN];

};



enum SendFrame_DstType 
{
    ReceiverDst,
    SenderDst
} SendFrame_DstType ;

typedef struct Sender_t Sender;
typedef struct Receiver_t Receiver;







//Declare global variables here
//DO NOT CHANGE: 
//   1) glb_senders_array
//   2) glb_receivers_array
//   3) glb_senders_array_length
//   4) glb_receivers_array_length
//   5) glb_sysconfig
//   6) CORRUPTION_BITS
Sender * glb_senders_array;
Receiver * glb_receivers_array;
int glb_senders_array_length;
int glb_receivers_array_length;
SysConfig glb_sysconfig;
int CORRUPTION_BITS;
#endif 
