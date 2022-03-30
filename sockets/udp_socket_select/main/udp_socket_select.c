#define udp_socket_select_c_

#include "udp_socket_select.h"


static const char *TAG = "udp_socket_select";

#if 1 //是否打印日志
#define my_printf ESP_LOGE
#else
#define my_printf(...)
#endif


#define CONFIG_EXAMPLE_IPV4

/**
  * @brief  select UDP任务
  * @param  none
  * @retval none
  */
void udp_socket_select_task(void *arg){

    pv_udp_socket_select_struct_t *pv_udp_socket_select_value = (pv_udp_socket_select_struct_t *)arg;
    uint8_t need_dns=0;

    struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_DGRAM,
    };
    struct addrinfo *res;

    socklen_t socklen;
    int len;
    int err;
    int socket_fd;
    char connection_state=0;
    struct sockaddr_in server_addr, local_addr;
    fd_set read_set;
    
    //尝试DNS解析,如果解析失败说明确实需要DNS解析,但是需要先连接路由器
    err = getaddrinfo(pv_udp_socket_select_value->remote_ip, NULL, &hints, &res);
    if(err != 0 || res == NULL) {
        my_printf(TAG, "DNS lookup failed err=%d res=%p", err, res);
        need_dns=1;
    }
    if (need_dns){//等待模组连接上路由器
        xEventGroupWaitBits(EventGroupHandleWiFiEvent, EventBitsWiFiStaConnected, false, true, portMAX_DELAY);
    }
    err = getaddrinfo(pv_udp_socket_select_value->remote_ip, NULL, &hints, &res);
    if(err != 0 || res == NULL) {
        my_printf(TAG, "DNS lookup failed err=%d res=%p", err, res);
        return;
    }

    /*本地地址配置*/
    local_addr.sin_family 			= res->ai_family;
    if (pv_udp_socket_select_value->local_port!=0){
        local_addr.sin_port   		= htons(pv_udp_socket_select_value->local_port);
    }
    

    /*对方地址配置*/
	server_addr.sin_family 			= res->ai_family;
	server_addr.sin_port   			= htons(pv_udp_socket_select_value->remote_port);
	server_addr.sin_addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
    freeaddrinfo(res);


    //创建socket
    socket_fd = socket(res->ai_family, res->ai_socktype, 0);
    if(socket_fd<0){
        my_printf(TAG, "Failed to allocate socket.");
    }
    //绑定socket
    err=bind(socket_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if (err<0){
        my_printf(TAG, "Failed to bind socket");
        close(socket_fd);
    }
    //清空fdset
    FD_ZERO(&read_set);
    //把sfd文件描述符添加到集合中
    FD_SET(socket_fd, &read_set);

    pv_udp_socket_select_value->socket_fd = socket_fd;

    // connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    while (true){
        err = select(socket_fd+1,&read_set,NULL,NULL,NULL);

        if(err < 0){
            my_printf(TAG, "select error \r\n");
            close(socket_fd);
            break;
        }

        if(FD_ISSET(socket_fd,&read_set))//有数据需要读取
        {
            socklen = sizeof(pv_udp_socket_select_value->sockaddr);
            err = recvfrom(socket_fd, pv_udp_socket_select_value->read_buff, sizeof(pv_udp_socket_select_value->read_buff), 0, (struct sockaddr *)&pv_udp_socket_select_value->sockaddr,&socklen);
            if(err>0){
                pv_udp_socket_select_value->read_buff[err]=0;
                pv_udp_socket_select_value->CallBack(pv_udp_socket_select_value->read_buff,err);
            }

            //  err = recvfrom(socket_fd, pv_udp_socket_select_value->read_buff, sizeof(pv_udp_socket_select_value->read_buff), MSG_PEEK, (struct sockaddr *)&pv_udp_socket_select_value->sockaddr,&socklen);

            // again:
            // len = read(socket_fd, pv_udp_socket_select_value->read_buff, sizeof(pv_udp_socket_select_value->read_buff));
            // if(len<0){
            //     if (errno == EINTR){
            //         goto again;
            //     }
            // }
            // if(len <= 0){
            //     close(socket_fd);
            // }else{//接收到客户端消息
            //     pv_udp_socket_select_value->read_buff[len]=0;
            //     pv_udp_socket_select_value->CallBack(pv_udp_socket_select_value->read_buff,len);

            //     // len = write(socket_fd, pv_tcp_client_select_value->read_buff, len);
            // }
        }
    }
    vTaskDelete(NULL);
}


/**
  * @brief  select UDP
  * @param  pv_udp_socket_select_value 客户端结构体
  * @param  task_priority 任务优先级
  * @param  RecvDataCallBack 接收回调函数
  * @retval none
  */
void udp_socket_select_task_init(pv_udp_socket_select_struct_t *pv_udp_socket_select_value, int task_priority,void(*RecvDataCallBack)(char *data,int len)){
    pv_udp_socket_select_value->socket_fd=-1;
    pv_udp_socket_select_value->CallBack = RecvDataCallBack;
    //创建TCP任务
    xTaskCreate(udp_socket_select_task, "tcp_client_select_task", 4096, pv_udp_socket_select_value , task_priority, NULL);
}


