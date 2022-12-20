#include "sender.h"   // sender.h,

void print_s_win(Sender_Windows *s_Win,int LAR,int LFS){
    fprintf(stderr,"\r\n——————打印当前窗口状态——————\r\n");
    for(int i = 0;i < SWS;++ i){
        fprintf(stderr,"窗口序号:[%d]\r\n",i);
        fprintf(stderr,"是否ACK: %d\r\n",s_Win[i].is_ack);
        fprintf(stderr,"帧内容:\r\n");
        print_frame(s_Win[i].frame);
    }
    fprintf(stderr,"\r\n——————打印当前窗口状态完毕——————\r\n");
}

void init_sender(Sender * sender, int id)
{
    //TODO: You should fill in this function as necessary
    // fprintf(stderr, "sender%d初始化开始\r\n",id);
    sender->send_id = id;
    sender->input_cmdlist_head = NULL;
    sender->input_framelist_head = NULL;
    //补充的地方
    sender->wait_cmdlist_head = NULL;
    sender->LAR = 0;
    sender->LFS = sender->LAR;
    struct timeval *init_time;
    init_time = (struct timeval *) malloc(sizeof(struct timeval));
    init_time->tv_sec = 0;
    for(int i = 0;i < SWS;++ i){
        sender->s_Win[i].time_val = init_time;
        sender->s_Win[i].is_ack = 0;
        sender->s_Win[i].frame = (Frame *) malloc(sizeof(Frame));
    }
    // fprintf(stderr, "sender%d初始化完成\r\n",id);
}

struct timeval * sender_get_next_expiring_timeval(Sender * sender)
{
    //TODO: You should fill in this function so that it returns the next timeout that should occur
    return NULL;
}


void handle_incoming_acks(Sender * sender,
                          LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling incoming ACKs
    //    1) Dequeue the ACK from the sender->input_framelist_head
    //    2) Convert the char * buffer to a Frame data type
    //    3) Check whether the frame is corrupted
    //    4) Check whether the frame is for this sender
    //    5) Do sliding window protocol for sender/receiver pair   
    // fprintf(stderr, "sender%d处理ACK\r\n",sender->send_id);
    
    int input_framelist_length = 0;
    input_framelist_length = ll_get_length(sender->input_framelist_head);

    // fprintf(stderr, "输入列表长度%d\r\n",input_framelist_length);
    while(input_framelist_length > 0){
        // fprintf(stderr,"出队列操作\r\n");
        
        LLnode * ack_node = ll_pop_node(&sender->input_framelist_head);
        input_framelist_length = ll_get_length(sender->input_framelist_head);


        // fprintf(stderr,"出队列操作完成\r\n");
        // fprintf(stderr,"——————发送者：处理ACK——————\r\n");
        char * raw_char_buf = (char*) ack_node->value;
        // fprintf(stderr,"封装成帧\r\n");
        Frame * inframe = convert_char_to_frame(raw_char_buf);
        // fprintf(stderr, "发送方receiver:%d 从接收方sender:%d获取序列号为seq:%d,确认ack为:%d的数据帧\n",sender->send_id,inframe->src_id,inframe->id,inframe->ack);
        // fprintf(stderr,"获取收到的ACK:%d\r\n",inframe->ack);
        //三个指针空间待释放
        free(raw_char_buf);//释放空间2
        
        //校验程序
        int result = is_correct(inframe);
        if(result){
            // fprintf(stderr,"判断ACK是否在发送者窗口内,如果在则移动窗口\r\n");
            // fprintf(stderr,"窗口移动前LAR:%d,LFE:%d\r\n",sender->LAR,sender->LFS);
            int in_Windows = is_in_window(inframe->ack,sender->LAR,sender->LFS);
            if(in_Windows){
                //累计确认
                // fprintf(stderr,"ack在窗口内\r\n");
                int i = sender->LAR + 1;
                for(i = sender->LAR + 1;i <= sender->LAR + SWS;i ++){
                    sender->s_Win[i%SWS].is_ack = 1;
                    // sender->s_Win[i%SWS].frame = NULL;
                    //到达ACk截断循环
                    if(i%MAX_LEN == inframe->ack) break;
                }
                sender->LAR = i%MAX_LEN;//移动LAR
            }
            // fprintf(stderr,"窗口移动完成LAR:%d,LFE:%d\r\n",sender->LAR,sender->LFS);
        }else{
            // fprintf(stderr,"crc wrong\r\n");
        }

        // print_s_win(sender->s_Win,sender->LAR,sender->LFS);
        free(inframe);
        free(ack_node);
        // fprintf(stderr,"空间释放完成\r\n");
        // fprintf(stderr,"——————发送者：ACK处理完毕——————\r\n");

    }
}


void handle_input_cmds(Sender * sender,
                       LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling input cmd
    //    1) Dequeue the Cmd from sender->input_cmdlist_head
    //    2) Convert to Frame
    //    3) Set up the frame according to the sliding window protocol
    //    4) Compute CRC and add CRC to Frame
    // fprintf(stderr,"处理发送者消息\r\n");
    int input_cmd_length = ll_get_length(sender->input_cmdlist_head);
    
        
    //Recheck the command queue length to see if stdin_thread dumped a command on us
    input_cmd_length = ll_get_length(sender->input_cmdlist_head);
    //等待队列长度
    
    int wait_cmdlist_length = ll_get_length(sender->wait_cmdlist_head);
    // fprintf(stderr,"开始处理信息\r\n");
    // (input_cmd_length > 0 || wait_cmdlist_length > 0)&&(sender->LAR != sender->LFS)
    while ((input_cmd_length > 0 || wait_cmdlist_length > 0)&&(sender->LFS != (sender->LAR+SWS)%MAX_LEN))
    {   
        // fprintf(stderr,"——————发送者：处理需要发送的消息——————\r\n");
        // fprintf(stderr,"窗口移动前LAR:%d,LFE:%d\r\n",sender->LAR,sender->LFS);
        LLnode * ll_input_cmd_node = NULL;
        if(wait_cmdlist_length > 0){
            //Pop a node off and update the input_cmd_length
            ll_input_cmd_node = ll_pop_node(&sender->wait_cmdlist_head);
            wait_cmdlist_length = ll_get_length(sender->wait_cmdlist_head);
        }else{
            ll_input_cmd_node = ll_pop_node(&sender->input_cmdlist_head);
            input_cmd_length = ll_get_length(sender->input_cmdlist_head);
        }
        //Cast to Cmd type and free up the memory for the node
        Cmd * outgoing_cmd = (Cmd *) ll_input_cmd_node->value;
        free(ll_input_cmd_node);
            

        //DUMMY CODE: Add the raw char buf to the outgoing_frames list
        //NOTE: You should not blindly send this message out!
        //      Ask yourself: Is this message actually going to the right receiver (recall that default behavior of send is to broadcast to all receivers)?
        //                    Does the receiver have enough space in in it's input queue to handle this message?
        //                    Were the previous messages sent to this receiver ACTUALLY delivered to the receiver?
        int msg_length = strlen(outgoing_cmd->message);
        if (msg_length > FRAME_PAYLOAD_SIZE - 1)
        {
            //Do something about messages that exceed the frame size
            //printf("<SEND_%d>: sending messages of length greater than %d is not implemented\n", sender->send_id, MAX_FRAME_SIZE);
            int message_size = FRAME_PAYLOAD_SIZE - 1;
            int divide_num = msg_length/message_size;
            if(msg_length%message_size > 0 ) divide_num ++;

            for(int i = 0;i < divide_num;i ++){
                Cmd * wait_cmd = (Cmd*) malloc (sizeof(Cmd));
                char * cmd_msg = (char*) malloc(sizeof(char)*FRAME_PAYLOAD_SIZE);
                strncpy(cmd_msg,outgoing_cmd->message+i*message_size,message_size);
                wait_cmd->src_id = outgoing_cmd->src_id;
                wait_cmd->dst_id = outgoing_cmd->dst_id;
                wait_cmd->message = (char*)malloc(sizeof(char)*FRAME_PAYLOAD_SIZE);
                strcpy(wait_cmd->message,cmd_msg);
                ll_append_node(&sender->wait_cmdlist_head,(void*)wait_cmd);
                
            }
        
        
        }
        else
        {
            outgoing_cmd->message[msg_length] = '\0';
            //This is probably ONLY one step you want
            // fprintf(stderr,"填充帧\r\n");

            sender->LFS = (sender->LFS + 1)%MAX_LEN;

            Frame * outgoing_frame = (Frame *) malloc (sizeof(Frame));
            strcpy(outgoing_frame->data, outgoing_cmd->message);
            outgoing_frame->src_id = outgoing_cmd->src_id;
            outgoing_frame->dst_id = outgoing_cmd->dst_id;
            outgoing_frame->id = sender->LFS;
            
            outgoing_frame->ack = sender->LFS;//发送者ACK无所谓
            outgoing_frame->length = strlen(outgoing_cmd->message);
            
            //计算校验码

            char * frame_char = convert_frame_to_char(outgoing_frame);
            uint16_t crc = getCRC16(frame_char,MAX_FRAME_SIZE-2);
            // fprintf(stderr,"加入CRC前\r\n");
            // print_frame(outgoing_frame);
            outgoing_frame->CRC = crc;
            // fprintf(stderr,"加入CRC后\r\n");
            // print_frame(outgoing_frame);
            free(frame_char);
            char * outgoing_charbuf = convert_frame_to_char(outgoing_frame);
            sender->s_Win[(sender->LFS)%SWS].frame = convert_char_to_frame(outgoing_charbuf);
            gettimeofday(sender->s_Win[(sender->LFS)%SWS].time_val,NULL);
            sender->s_Win[(sender->LFS)%SWS].is_ack = 0;
            // sender->buffer[(sender->LFS)] = *(convert_char_to_frame(outgoing_charbuf));
            //At this point, we don't need the outgoing_cmd
            
            // fprintf(stderr,"填充完毕\r\n");
            //Convert the message to the outgoing_charbuf
            
            ll_append_node(outgoing_frames_head_ptr,outgoing_charbuf);
            free(outgoing_frame);
            free(outgoing_cmd->message);
            free(outgoing_cmd);
            
        }
        // fprintf(stderr,"窗口移动完成LAR:%d,LFE:%d\r\n",sender->LAR,sender->LFS);
        print_s_win(sender->s_Win,sender->LAR,sender->LFS);
        // fprintf(stderr,"——————发送者：发送信息处理完毕——————\r\n"); 
        
    }  
    
}


void handle_timedout_frames(Sender * sender,
                            LLnode ** outgoing_frames_head_ptr)
{
    //TODO: Suggested steps for handling timed out datagrams
    //    1) Iterate through the sliding window protocol information you maintain for each receiver
    //    2) Locate frames that are timed out and add them to the outgoing frames
    //    3) Update the next timeout field on the outgoing frames
    
    int i = 0;
    if(sender->LAR == sender->LFS) return;
    for(i = sender->LAR + 1;i <= sender->LAR+SWS;++ i){
        int i_temp = i%SWS;
        
        long dif = 0;
        struct timeval nowtime;
        gettimeofday(&nowtime,NULL);

        if(sender->s_Win[i_temp].is_ack == 0){
            
            dif = timeval_usecdiff(sender->s_Win[i_temp].time_val,&nowtime);
            if(dif > 100000){
                // fprintf(stderr,"——————发送者：处理超时帧——————\r\n"); 
                // fprintf(stderr,"该帧超时,在窗口位置为%d:\r\n",i_temp);
                // print_frame(sender->s_Win[i_temp].frame);
                // fprintf(stderr,"——————发送者：处理超时帧完毕——————\r\n");
                char * outgoing_charbuf = convert_frame_to_char(sender->s_Win[i_temp].frame);
                ll_append_node(outgoing_frames_head_ptr,outgoing_charbuf);
            }
             
        }
        //到达上界直接截断
        if(i%MAX_LEN == sender->LFS) break;
    }
}


void * run_sender(void * input_sender)
{    
    struct timespec   time_spec;
    struct timeval    curr_timeval;
    const int WAIT_SEC_TIME = 0;
    const long WAIT_USEC_TIME = 100000;
    Sender * sender = (Sender *) input_sender;    
    LLnode * outgoing_frames_head;
    struct timeval * expiring_timeval;
    long sleep_usec_time, sleep_sec_time;
    
    //This incomplete sender thread, at a high level, loops as follows:
    //1. Determine the next time the thread should wake up
    //2. Grab the mutex protecting the input_cmd/inframe queues
    //3. Dequeues messages from the input queue and adds them to the outgoing_frames list
    //4. Releases the lock
    //5. Sends out the messages

    pthread_cond_init(&sender->buffer_cv, NULL);
    pthread_mutex_init(&sender->buffer_mutex, NULL);

    while(1)
    {    
        outgoing_frames_head = NULL;

        //Get the current time
        gettimeofday(&curr_timeval, 
                     NULL);

        //time_spec is a data structure used to specify when the thread should wake up
        //The time is specified as an ABSOLUTE (meaning, conceptually, you specify 9/23/2010 @ 1pm, wakeup)
        time_spec.tv_sec  = curr_timeval.tv_sec;
        time_spec.tv_nsec = curr_timeval.tv_usec * 1000;

        //Check for the next event we should handle
        expiring_timeval = sender_get_next_expiring_timeval(sender);

        //Perform full on timeout
        if (expiring_timeval == NULL)
        {
            time_spec.tv_sec += WAIT_SEC_TIME;
            time_spec.tv_nsec += WAIT_USEC_TIME * 1000;
        }
        else
        {
            //Take the difference between the next event and the current time
            sleep_usec_time = timeval_usecdiff(&curr_timeval,
                                               expiring_timeval);

            //Sleep if the difference is positive
            if (sleep_usec_time > 0)
            {
                sleep_sec_time = sleep_usec_time/1000000;
                sleep_usec_time = sleep_usec_time % 1000000;   
                time_spec.tv_sec += sleep_sec_time;
                time_spec.tv_nsec += sleep_usec_time*1000;
            }   
        }

        //Check to make sure we didn't "overflow" the nanosecond field
        if (time_spec.tv_nsec >= 1000000000)
        {
            time_spec.tv_sec++;
            time_spec.tv_nsec -= 1000000000;
        }

        
        //*****************************************************************************************
        //NOTE: Anything that involves dequeing from the input frames or input commands should go 
        //      between the mutex lock and unlock, because other threads CAN/WILL access these structures
        //*****************************************************************************************
        pthread_mutex_lock(&sender->buffer_mutex);

        //Check whether anything has arrived
        int input_cmd_length = ll_get_length(sender->input_cmdlist_head);
        int inframe_queue_length = ll_get_length(sender->input_framelist_head);
        
        //Nothing (cmd nor incoming frame) has arrived, so do a timed wait on the sender's condition variable (releases lock)
        //A signal on the condition variable will wakeup the thread and reaquire the lock
        if (input_cmd_length == 0 &&
            inframe_queue_length == 0)
        {
            
            pthread_cond_timedwait(&sender->buffer_cv, 
                                   &sender->buffer_mutex,
                                   &time_spec);
        }
        //Implement this
        handle_incoming_acks(sender,
                             &outgoing_frames_head);
        // fprintf(stderr,"完成ACK处理\r\n");
        //Implement this
        handle_input_cmds(sender,
                          &outgoing_frames_head);
        // fprintf(stderr,"完成cmd处理\r\n");

        pthread_mutex_unlock(&sender->buffer_mutex);


        //Implement this
        // fprintf(stderr,"开始超时处理\r\n");
        handle_timedout_frames(sender,
                               &outgoing_frames_head);
        // fprintf(stderr,"完成超时处理\r\n");

        //CHANGE THIS AT YOUR OWN RISK!
        //Send out all the frames
        int ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        // fprintf(stderr,"开始发送\r\n");
        while(ll_outgoing_frame_length > 0)
        {
            LLnode * ll_outframe_node = ll_pop_node(&outgoing_frames_head);
            char * char_buf = (char *)  ll_outframe_node->value;

            //Don't worry about freeing the char_buf, the following function does that
            send_msg_to_receivers(char_buf);

            //Free up the ll_outframe_node
            free(ll_outframe_node);

            ll_outgoing_frame_length = ll_get_length(outgoing_frames_head);
        }
        // fprintf(stderr,"发送完成\r\n");
    }
    pthread_exit(NULL);
    return 0;
}
