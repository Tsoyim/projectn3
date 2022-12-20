
#include "receiver.h"   // comment receiver.h,





void init_receiver(Receiver * receiver,
                   int id)
{
    receiver->recv_id = id;
    receiver->input_framelist_head = NULL;

    receiver->NFE = 1;
    receiver->lastACK = 0;
    for(int i = 0;i < SWS;++ i){
        receiver->r_Win[i].frame = NULL;
        receiver->r_Win[i].frame = (Frame *) malloc(sizeof(Frame));
    }
}


void handle_incoming_msgs(Receiver * receiver,
                          LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling incoming frames
    //    1) Dequeue the Frame from the sender->input_framelist_head
    //    2) Convert the char * buffer to a Frame data type
    //    3) Check whether the frame is corrupted
    //    4) Check whether the frame is for this receiver
    //    5) Do sliding window protocol for sender/receiver pair

    int incoming_msgs_length = ll_get_length(receiver->input_framelist_head);
    while (incoming_msgs_length > 0)
    {
        // fprintf(stderr,"——————接收者：处理发送过来的消息——————\r\n");
        //Pop a node off the front of the link list and update the count
        LLnode * ll_inmsg_node = ll_pop_node(&receiver->input_framelist_head);
        incoming_msgs_length = ll_get_length(receiver->input_framelist_head);

        //DUMMY CODE: Print the raw_char_buf
        //NOTE: You should not blindly print messages!
        //      Ask yourself: Is this message really for me?
        //                    Is this message corrupted?
        //                    Is this an old, retransmitted message?           
        char * raw_char_buf = (char *) ll_inmsg_node->value;
        Frame * inframe = convert_char_to_frame(raw_char_buf);
        // print_frame(inframe);
        //Free raw_char_buf
        // free(raw_char_buf);
        // fprintf(stderr,"接收者NFE：%d,帧ID:%d\r\n",receiver->NFE,inframe->id);
        //校验码、稍后再敲
        int result = is_correct(inframe);
        if(result){
            if(inframe->dst_id != receiver->recv_id){
                // fprintf(stderr,"id错误\r\n");
                continue;
            }
            if(inframe->id != receiver->NFE){
                Frame *outgoing_frames = (Frame*)malloc(sizeof(Frame));//获得空间
                outgoing_frames->ack = receiver->lastACK;
                outgoing_frames->id = receiver->lastACK;
                outgoing_frames->src_id = receiver->recv_id;
                outgoing_frames->dst_id = inframe->src_id;
                //CRC校验码
                char * frame_char = convert_frame_to_char(outgoing_frames);
                uint16_t crc = getCRC16(frame_char,MAX_FRAME_SIZE-2);
                outgoing_frames->CRC = crc;
                free(frame_char);

                char *outgoint_charbuf = convert_frame_to_char(outgoing_frames);
                ll_append_node(outgoing_frames_head_ptr,outgoint_charbuf);
                free(outgoing_frames);//释放空间
            }
            int r = is_in_rec_window(inframe->id,receiver->NFE);
            if(r == 1&&receiver->r_Win[inframe->id%RWS].received == 0)
            {
                receiver->r_Win[inframe->id%RWS].received = 1;
                receiver->r_Win[inframe->id%RWS].frame = convert_char_to_frame(raw_char_buf);
                // strcpy(receiver->buffer[inframe->id].data,inframe->data);

                while(receiver->r_Win[receiver->NFE%RWS].received == 1){
                    // fprintf(stderr,"打印已接收的消息\n");
                    fprintf(stderr, "接收方receiver:%d 从发送方sender:%d获取序列号为seq:%d，确认ack为:%d的数据帧\n", receiver->recv_id,inframe->src_id,inframe->id,inframe->ack);
                    fprintf(stderr,"<RECV%d>:[%s]\n", receiver->recv_id, receiver->r_Win[receiver->NFE%RWS].frame->data);
                    printf("<RECV%d>:[%s]\n", receiver->recv_id, receiver->r_Win[receiver->NFE%RWS].frame->data);
                    // fprintf(stderr,"打印完成\n");
                    //滑动窗口
                    
                    //发送ACK确认
                    Frame *outgoing_frames = (Frame*)malloc(sizeof(Frame));//获得空间
                    outgoing_frames->ack = receiver->NFE;
                    outgoing_frames->id = receiver->NFE;
                    receiver->lastACK = receiver->NFE;
                    receiver->NFE = (receiver->NFE + 1)%MAX_LEN;
                    outgoing_frames->src_id = receiver->recv_id;
                    outgoing_frames->dst_id = inframe->src_id;
                    
                    
                    //把位置置空
                    receiver->r_Win[receiver->NFE%RWS].received = 0;
                    //CRC校验码
                    char * frame_char = convert_frame_to_char(outgoing_frames);
                    uint16_t crc = getCRC16(frame_char,MAX_FRAME_SIZE-2);
                    outgoing_frames->CRC = crc;
                    free(frame_char);

                    char *outgoint_charbuf = convert_frame_to_char(outgoing_frames);
                    ll_append_node(outgoing_frames_head_ptr,outgoint_charbuf);
                    free(outgoing_frames);//释放空间

                }

            }
        }else{
            fprintf(stderr,"crc wrong\n");
            // Frame *outgoing_frames = (Frame*)malloc(sizeof(Frame));//获得空间
            // outgoing_frames->ack = (receiver->NFE + MAX_LEN)%(MAX_LEN+1);
            // outgoing_frames->id = (receiver->NFE + MAX_LEN)%(MAX_LEN+1);
            // outgoing_frames->src_id = receiver->recv_id;
            // outgoing_frames->dst_id = inframe->src_id;
            // //把位置置空
            // receiver->r_Win[receiver->NFE%RWS].received = 0;
            // //CRC校验码
            // char * frame_char = convert_frame_to_char(outgoing_frames);
            // uint16_t crc = getCRC16(frame_char,MAX_FRAME_SIZE-2);
            // outgoing_frames->CRC = crc;
            // free(frame_char);

            // char *outgoint_charbuf = convert_frame_to_char(outgoing_frames);
            // ll_append_node(outgoing_frames_head_ptr,outgoint_charbuf);
            // free(outgoing_frames);//释放空间
        }
        // fprintf(stderr,"接收者NFE：%d\r\n",receiver->NFE);
        
        
        free(raw_char_buf);
        free(inframe);
        free(ll_inmsg_node);
        // fprintf(stderr,"——————接收者：接收信息处理完毕——————\r\n");
    }
    
}

void * run_receiver(void * input_receiver)
{    
    struct timespec   time_spec;
    struct timeval    curr_timeval;
    const int WAIT_SEC_TIME = 0;
    const long WAIT_USEC_TIME = 100000;
    Receiver * receiver = (Receiver *) input_receiver;
    LLnode * outgoing_frames_head;


    //This incomplete receiver thread, at a high level, loops as follows:
    //1. Determine the next time the thread should wake up if there is nothing in the incoming queue(s)
    //2. Grab the mutex protecting the input_msg queue
    //3. Dequeues messages from the input_msg queue and prints them
    //4. Releases the lock
    //5. Sends out any outgoing messages

    pthread_cond_init(&receiver->buffer_cv, NULL);
    pthread_mutex_init(&receiver->buffer_mutex, NULL);

    while(1)
    {    
        //NOTE: Add outgoing messages to the outgoing_frames_head pointer
        outgoing_frames_head = NULL;
        gettimeofday(&curr_timeval, 
                     NULL);

        //Either timeout or get woken up because you've received a datagram
        //NOTE: You don't really need to do anything here, but it might be useful for debugging purposes to have the receivers periodically wakeup and print info
        time_spec.tv_sec  = curr_timeval.tv_sec;
        time_spec.tv_nsec = curr_timeval.tv_usec * 1000;
        time_spec.tv_sec += WAIT_SEC_TIME;
        time_spec.tv_nsec += WAIT_USEC_TIME * 1000;
        if (time_spec.tv_nsec >= 1000000000)
        {
            time_spec.tv_sec++;
            time_spec.tv_nsec -= 1000000000;
        }

        //*****************************************************************************************
        //NOTE: Anything that involves dequeing from the input frames should go 
        //      between the mutex lock and unlock, because other threads CAN/WILL access these structures
        //*****************************************************************************************
        pthread_mutex_lock(&receiver->buffer_mutex);

        //Check whether anything arrived
        int incoming_msgs_length = ll_get_length(receiver->input_framelist_head);
        if (incoming_msgs_length == 0)
        {
            //Nothing has arrived, do a timed wait on the condition variable (which releases the mutex). Again, you don't really need to do the timed wait.
            //A signal on the condition variable will wake up the thread and reacquire the lock
            pthread_cond_timedwait(&receiver->buffer_cv, 
                                   &receiver->buffer_mutex,
                                   &time_spec);
        }

        handle_incoming_msgs(receiver,
                             &outgoing_frames_head);

        pthread_mutex_unlock(&receiver->buffer_mutex);
        
        //CHANGE THIS AT YOUR OWN RISK!
        //Send out all the frames user has appended to the outgoing_frames list
        int ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        while(ll_outgoing_frame_length > 0)
        {
            LLnode * ll_outframe_node = ll_pop_node(&outgoing_frames_head);
            char * char_buf = (char *) ll_outframe_node->value;
            
            //The following function frees the memory for the char_buf object
            send_msg_to_senders(char_buf);

            //Free up the ll_outframe_node
            free(ll_outframe_node);

            ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        }
    }
    pthread_exit(NULL);

}
