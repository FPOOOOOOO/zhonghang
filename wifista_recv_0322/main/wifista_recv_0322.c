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
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY

#define PORT CONFIG_EXAMPLE_PORT

static const char *payload = "Message from ESP32 ";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
static esp_netif_t *sta_netif = NULL;
esp_ip4_addr_t stationip[16];
uint16_t port[16];
char read_buff[1500]; //接收数据的缓存数组
static uint16_t recv_header = 0x0000;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

// /**< RX callback function in the promiscuous mode. */
// static void wifi_sniffer_cb(void *recv_buf, wifi_promiscuous_pkt_type_t type)
// {
//     if (type != WIFI_PKT_MGMT) {
//         return;
//     }

//     wifi_promiscuous_pkt_t *promiscuous_pkt = (wifi_promiscuous_pkt_t *)recv_buf;
//     uint8_t *payload                        = promiscuous_pkt->payload;

//     for (int i = 36; i < promiscuous_pkt->rx_ctrl.sig_len; i += payload[i + 1] + 2) {
//         vendor_ie_data_t *ie_data = (vendor_ie_data_t *)(payload + i);

//         if (ie_data->element_id == WIFI_VENDOR_IE_ELEMENT_ID && ie_data->length == 4
//                 && !memcmp(ie_data->vendor_oui, MDF_VENDOR_OUI, sizeof(ie_data->vendor_oui))
//                 && ie_data->vendor_oui_type == VENDOR_OUI_TYPE_CONFIG) {
//             mconfig_scan_info_t scan_info = {0};

//             /**< If g_mconfig_scan_queue is full, delete the front item */
//             if (!uxQueueSpacesAvailable(g_mconfig_scan_queue)) {
//                 xQueueReceive(g_mconfig_scan_queue, &scan_info, 0);
//             }

//             scan_info.rssi = promiscuous_pkt->rx_ctrl.rssi;
//             memcpy(scan_info.bssid, payload + 10, MWIFI_ADDR_LEN);
//             xQueueSend(g_mconfig_scan_queue, &scan_info, 0);
//         }
//     }
// }

static esp_err_t wifi2pkg(void *buffer, uint16_t len, void *eb)
{
    ESP_LOGI(TAG, "LEN =: %d", len);
    memcpy((uint16_t *)&recv_header, buffer, 2);
    recv_header = ntohs(recv_header);
    ESP_LOGI(TAG, "RECV_HEADER IS: %x", recv_header);

    if (len > 42)
    {
        memcpy((esp_ip4_addr_t *)&stationip[0], buffer + 26, 4); // source ip
        memcpy((uint16_t *)&port[0], buffer + 36, 2);            // source port
        memcpy(read_buff, buffer + 42, len - 42);                // source data
        ESP_LOGI(TAG, "Recv socket by WIFI: %s", read_buff);
        ESP_LOGI(TAG, "This message is from:%s", ip4addr_ntoa((ip4_addr_t *)&stationip[0]));
        ESP_LOGI(TAG, "Port is:%d", ntohs(port[0]));
    }
    else
    {
        if (recv_header == 0xA55A)
        {
            uint8_t *wifi_data = (uint8_t *)malloc(len - 11);
            memcpy(wifi_data, buffer + 9, len - 11);
            //     recv_data.header=recv_header;
            //     memcpy((uint8_t *)&(recv_data.type),buffer+2,1);
            //     memcpy((uint32_t *)&(recv_data.dest),buffer+3,3);
            //     memcpy((uint8_t *)&(recv_data.source),buffer+6,1);
            //     memcpy((uint16_t *)&(recv_data.len),buffer+7,2);
            //     memcpy(wifi_data,buffer+9,len-11);
            //     recv_data.buffer = wifi_data;
            //     memcpy((uint16_t *)&(recv_data.CRC),buffer+len-2,2);
            //     if(recv_data.type==UART){
            //         uart_write_bytes(EX_UART_NUM, (const char *)recv_data.buffer, recv_data.len-11);
            //     }

            for (int i = 0; i < len - 11; i++)
            {
                printf("%c", ((char *)wifi_data)[i]);
            }
            free(wifi_data);
            printf("\n\r");
        }
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

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            esp_wifi_internal_reg_rxcb(WIFI_IF_STA, NULL);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED\n\r");
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_internal_reg_rxcb(WIFI_IF_STA, wifi2pkg);
        // ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(wifi_sniffer_cb));
        // ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    // sta_netif=netif;

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
                .required = false},
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    // esp_wifi_set_promiscuous(true);
    ESP_ERROR_CHECK(esp_wifi_start());

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
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    esp_netif_dhcpc_stop(netif);
    char *ap_ip = "192.168.4.7";
    char *ap_gateway = "192.168.4.1";
    char *ap_netmask = "255.255.255.0";
    esp_netif_ip_info_t info_t;
    memset(&info_t, 0, sizeof(esp_netif_ip_info_t));
    ip4addr_aton((const char *)ap_ip, &info_t.ip.addr);
    ip4addr_aton((const char *)ap_gateway, &info_t.gw.addr);
    ip4addr_aton((const char *)ap_netmask, &info_t.netmask.addr);
    esp_netif_set_ip_info(netif, &info_t);
    ESP_LOGI(TAG, "wifi_sta ip is %s.", ip4addr_ntoa((ip4_addr_t *)&info_t.ip));

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

static void udp_task(void *pvParameters)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    while (1)
    {

        if (addr_family == AF_INET)
        {
            struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
            dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
            dest_addr_ip4->sin_family = AF_INET;
            dest_addr_ip4->sin_port = htons(PORT);
            ip_protocol = IPPROTO_IP;
        }
        else if (addr_family == AF_INET6)
        {
            bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
            dest_addr.sin6_family = AF_INET6;
            dest_addr.sin6_port = htons(PORT);
            ip_protocol = IPPROTO_IPV6;
        }

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
        if (addr_family == AF_INET6)
        {
            // Note that by default IPV6 binds to both protocols, it is must be disabled
            // if both protocols used at the same time (used in CI)
            int opt = 1;
            setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
        }
#endif

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0)
        {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        while (1)
        {

            // ESP_LOGI(TAG, "Waiting for data");
            struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
            socklen_t socklen = sizeof(source_addr);
            ESP_LOGI(TAG, "I am here");
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            ESP_LOGI(TAG, "I am here 2");
            // Error occurred during receiving
            if (len < 0)
            {
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            // Data received
            else
            {
                // Get the sender's ip address as string
                if (source_addr.ss_family == PF_INET)
                {
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
                }
                else if (source_addr.ss_family == PF_INET6)
                {
                    inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
                ESP_LOGI(TAG, "Received %d bytes by UDP from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);

                // int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                // if (err < 0) {
                //     ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                //     break;
                // }
            }
        }

        if (sock != -1)
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

#ifdef CONFIG_EXAMPLE_IPV4
    xTaskCreate(udp_task, "udp_server", 4096, (void *)AF_INET, 5, NULL);
#endif
#ifdef CONFIG_EXAMPLE_IPV6
    xTaskCreate(udp_task, "udp_server", 4096, (void *)AF_INET6, 5, NULL);
#endif
}
