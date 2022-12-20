#include "util.h"

//Linked list functions
int ll_get_length(LLnode * head)
{
    LLnode * tmp;
    int count = 1;
    if (head == NULL)
        return 0;
    else
    {
        tmp = head->next;
        while (tmp != head)
        {
            count++;
            tmp = tmp->next;
        }
        return count;
    }
}

void ll_append_node(LLnode ** head_ptr, 
                    void * value)
{
    LLnode * prev_last_node;
    LLnode * new_node;
    LLnode * head;

    if (head_ptr == NULL)
    {
        return;
    }
    
    //Init the value pntr
    head = (*head_ptr);
    new_node = (LLnode *) malloc(sizeof(LLnode));
    new_node->value = value;

    //The list is empty, no node is currently present
    if (head == NULL)
    {
        (*head_ptr) = new_node;
        new_node->prev = new_node;
        new_node->next = new_node;
    }
    else
    {
        //Node exists by itself
        prev_last_node = head->prev;
        head->prev = new_node;
        prev_last_node->next = new_node;
        new_node->next = head;
        new_node->prev = prev_last_node;
    }
}


LLnode * ll_pop_node(LLnode ** head_ptr)
{
    LLnode * last_node;
    LLnode * new_head;
    LLnode * prev_head;

    prev_head = (*head_ptr);
    if (prev_head == NULL)
    {
        return NULL;
    }
    last_node = prev_head->prev;
    new_head = prev_head->next;

    //We are about to set the head ptr to nothing because there is only one thing in list
    if (last_node == prev_head)
    {
        (*head_ptr) = NULL;
        prev_head->next = NULL;
        prev_head->prev = NULL;
        return prev_head;
    }
    else
    {
        (*head_ptr) = new_head;
        last_node->next = new_head;
        new_head->prev = last_node;

        prev_head->next = NULL;
        prev_head->prev = NULL;
        return prev_head;
    }
}

void ll_destroy_node(LLnode * node)
{
    if (node->type == llt_string)
    {
        free((char *) node->value);
    }
    free(node);
}

//Compute the difference in usec for two timeval objects
long timeval_usecdiff(struct timeval *start_time, 
                      struct timeval *finish_time)
{
  long usec;
  usec=(finish_time->tv_sec - start_time->tv_sec)*1000000;
  usec+=(finish_time->tv_usec- start_time->tv_usec);
  return usec;
}


//Print out messages entered by the user
void print_cmd(Cmd * cmd)
{
    fprintf(stderr, "src=%d, dst=%d, message=%s\n", 
           cmd->src_id,
           cmd->dst_id,
           cmd->message);
}


char * convert_frame_to_char(Frame * frame)
{
    //TODO: You should implement this as necessary
    char * char_buffer = (char *) malloc(MAX_FRAME_SIZE);
    memset(char_buffer,
           0,
           MAX_FRAME_SIZE);
    memcpy(char_buffer, 
           frame,
           MAX_FRAME_SIZE);
    return char_buffer;
}


Frame * convert_char_to_frame(char * char_buf)
{
    //TODO: You should implement this as necessary
    Frame * frame = (Frame *) malloc(sizeof(Frame));
    memset(frame,
           0,
           sizeof(char)*sizeof(Frame));
    memcpy(frame, 
           char_buf,
           sizeof(char)*sizeof(Frame));
    return frame;
}

// Frame * copy_frame(Frame *inframe)
// {
//     //TODO: You should implement this as necessary
//     Frame * frame = (Frame *) malloc(sizeof(Frame));
//     memset(inframe,
//            0,
//            sizeof(char)*sizeof(Frame));
//     memcpy(frame, 
//            inframe,
//            sizeof(char)*sizeof(Frame));
//     return frame;
// }


//补充函数
//补充函数
int is_in_window(int id,int LAR,int LFS){
    if (LAR < LFS) { 
        if(id > LAR && id <= LFS){
            return 1;
        }
    }
    if(LAR > LFS){
        if(id > LAR || id <= LFS){
            return 1;
        }
    }
    return 0;
}
//接收窗口
int is_in_rec_window(int id,int NFE){
    if(NFE + RWS < MAX_LEN + 1){
        if(id >= NFE && id < NFE + RWS){
            return 1;
        }
    }else{
        if(id >= NFE || id < (NFE + RWS)%MAX_LEN){
            return 1;
        }
    }
    return 0;
}
//打印帧
void print_frame(Frame* frame)
{
    fprintf(stderr, "\nframe:\n");

    fprintf(stderr, "frame->id=%d\n", frame->id);
    fprintf(stderr, "frame->CRC=%d\n", frame->CRC);
    fprintf(stderr, "frame->dst_id=%d\n", frame->dst_id);
    fprintf(stderr, "frame->src_id=%d\n", frame->src_id);
    fprintf(stderr, "frame->data=%s\n", frame->data);
    fprintf(stderr, "#frame--------\n\n");
}
//计算CRC16

uint16_t getCRC16(char *ptr,uint8_t len)
{
		int i;
		uint16_t crc = 0xFFFF;
		
		if(ptr == 0)
		{
				return 0;
		}
		if(len==0)
		{
			len = 1;
		}
        while(len--)
        {
                crc ^= *ptr;
                for(i=0; i<8; i++)
                {
                    if(crc&1)
                    {
                        crc >>= 1;
                        crc ^= 0xA001;
                    }
                    else
                        crc >>= 1;
                }
                ptr++;
        }
		return(crc);
}
//判断是否正确
int is_correct(Frame *frame){
    char * char_arr = convert_frame_to_char(frame);
    uint16_t crc = frame->CRC;
    if(crc == getCRC16(char_arr,MAX_FRAME_SIZE-2)) return 1;
    free(char_arr);
    return 0;
}
