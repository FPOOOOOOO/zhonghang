#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "wifi_softap_sta.h"
#include "usart.h"
#include "udp_socket_select.h"

PRIVILEGED_DATA static portMUX_TYPE xTaskQueueMutex = portMUX_INITIALIZER_UNLOCKED;

pv_udp_socket_select_struct_t udp_client;//udp 结构体变量


//udp接收数据回调函数
void udp_recv_data(char *data,int len){
  //获取对方地址,使用
  char addr_str[128];
  inet_ntoa_r(((struct sockaddr_in *)&udp_client.sockaddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
  //获取对方端口号 (udp_client.sockaddr.sin_port 由于系统数据存储大小端问题,需要做下转换)
  uint16_t port = ntohs(udp_client.sockaddr.sin_port);
  //打印对方的IP地址和端口号
  ESP_LOGE("main", "udp socket addr=%s  %d\r\n",addr_str, port);

  // /*发送数据给UDP*/
  // struct sockaddr_in dest_addr;
  // dest_addr.sin_addr.s_addr = inet_addr("192.168.0.104");//对方地址
  // dest_addr.sin_family = AF_INET;
  // dest_addr.sin_port = htons(1000);//对方端口号
  // sendto(udp_client.socket_fd, data, len, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));

  /*返回数据给UDP*/
  sendto(udp_client.socket_fd, data, len, 0, (struct sockaddr*)&udp_client.sockaddr, sizeof(udp_client.sockaddr));
}


void app_main(void){
    //初始化 NVS(配置WiFi的参数存储需要用到NVS)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //初始化内部的lwip
    ESP_ERROR_CHECK(esp_netif_init());
    //创建系统事件任务
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //初始化配置AP+STA
    wifi_init_softap_sta();

    portENTER_CRITICAL(&xTaskQueueMutex);//进入临界点
    usart_init(20);

    //strcpy(udp_client.ip, "mnif.cn");//设置对方的IP地址
    strcpy(udp_client.remote_ip, "192.168.0.104");//设置对方的IP地址
    udp_client.remote_port = 1000;//设置对方的端口号

    udp_client.local_port = 2000;//设置本地的端口号(如果设置为0 则程序会自动分配)
    udp_socket_select_task_init(&udp_client,10,udp_recv_data);

    portEXIT_CRITICAL(&xTaskQueueMutex);//退出临界点
}