/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "esp_eth.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_private/wifi.h"
#include "driver/gpio.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_system.h"
#include "/Users/fpo/esp/esp-idf/components/lwip/lwip/src/include/lwip/ip4_addr.h"

#include "freertos/event_groups.h"

#include "sys/param.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>
#include "/Users/fpo/esp/esp-idf/examples/common_components/protocol_examples_common/include/addr_from_stdin.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
static esp_netif_t *sta_netif = NULL;
esp_ip4_addr_t stationip[16];
uint16_t port[16];
char read_buff[1500];//接收数据的缓存数组 

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

static esp_err_t wifi2pkg(void *buffer, uint16_t len, void *eb)
{
    if(len > 42){
        memcpy((esp_ip4_addr_t *)&stationip[0],buffer+26,4);//source ip
        memcpy((uint16_t *)&port[0],buffer+36,2);//source port
        memcpy(read_buff,buffer+42,len-42);//source data
        ESP_LOGI(TAG, "Recv from UDP socket: %s",read_buff);
        ESP_LOGI(TAG, "This message is from:%s",ip4addr_ntoa((ip4_addr_t *)&stationip[0]));
        ESP_LOGI(TAG, "Portis:%d",ntohs(port[0]));
    }else{
        ESP_LOGI(TAG, "Recv from wifi: %s",(char *)buffer);
    }
    
    // if (s_ethernet_is_connected)
    // {
    //     //printf("pkt_wifi2eth is here\n\r ");
    //     if (esp_eth_transmit(s_eth_handle, buffer, len) != ESP_OK)
    //     {
    //         ESP_LOGE(TAG, "Ethernet send packet failed");
    //     }
    // }

    // for(int i=0;i<len;i++){
    //     printf("%c",((char*)buffer)[i]);
    // }
    // printf("\n\r");
    esp_wifi_internal_free_rx_buffer(eb);
    return ESP_OK;
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            esp_wifi_internal_reg_rxcb(WIFI_IF_STA, NULL);
            
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED){
        ESP_LOGI(TAG,"WIFI_EVENT_STA_CONNECTED\n\r");
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_internal_reg_rxcb(WIFI_IF_STA, wifi2pkg);
        
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    //sta_netif=netif;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    esp_netif_dhcpc_stop(netif);
    char* ap_ip= "192.168.4.7";
    char* ap_gateway = "192.168.4.1";
    char* ap_netmask = "255.255.255.0";
    esp_netif_ip_info_t info_t;
    memset(&info_t, 0, sizeof(esp_netif_ip_info_t));
    ip4addr_aton((const char *)ap_ip, &info_t.ip.addr);
    ip4addr_aton((const char *)ap_gateway, &info_t.gw.addr);
    ip4addr_aton((const char *)ap_netmask, &info_t.netmask.addr);
    esp_netif_set_ip_info(netif, &info_t);
    ESP_LOGI(TAG, "wifi_sta ip is %s.",ip4addr_ntoa((ip4_addr_t*)&info_t.ip));

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
}
