#ifndef udp_socket_select_h_
#define udp_socket_select_h_

#ifndef udp_socket_select_c_
#define udp_socket_select_cx_ extern
#else
#define udp_socket_select_cx_
#endif


#include <stdio.h>
#include <string.h>
#include <sys/select.h>
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
#include "wifi_softap_sta.h"

#include "public_variable.h"

void udp_socket_select_task_init(pv_udp_socket_select_struct_t *pv_udp_socket_select_value, int task_priority,void(*RecvDataCallBack)(char *data,int len));

#endif