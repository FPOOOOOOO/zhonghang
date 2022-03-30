/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN CONFIG_ESP_MAX_STA_CONN

// #define PORT[0] CONFIG_EXAMPLE_PORT1
// #define PORT[1] CONFIG_EXAMPLE_PORT2
// #define PORT[2] CONFIG_EXAMPLE_PORT3

static const char *TAG = "udps_softAP";
static esp_netif_t *ap_netif = NULL;
esp_netif_ip_info_t ip_info;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *netif = esp_netif_create_default_wifi_ap();
    ap_netif = netif;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);

    // esp_netif_config_t netif_config = ESP_NETIF_DEFAULT_WIFI_AP();
    // esp_netif_t *netif = esp_netif_new(&netif_config);
    // assert(netif);
    // esp_netif_attach_wifi_ap(netif);
    // esp_wifi_set_default_wifi_ap_handlers();

    esp_netif_get_ip_info(ap_netif, &ip_info);
    ESP_LOGI(TAG, "THis AP has got an IP as:%s", ip4addr_ntoa((ip4_addr_t *)&ip_info.ip));
}

static void udp_server_task(void *pvParameters)
{
    char rx_buffer[3][128];
    char addr_str[3][128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int sock[3];
    int PORT[3];
    struct sockaddr_in6 dest_addr[3];

    PORT[0] = CONFIG_EXAMPLE_PORT1;
    PORT[1] = CONFIG_EXAMPLE_PORT2;
    PORT[2] = CONFIG_EXAMPLE_PORT3;

    while (1)
    {

        for (int i = 0; i < 3; i++)
        {
            sock[i] = socket(addr_family, SOCK_DGRAM, ip_protocol);
            if (sock[i] < 0)
            {
                ESP_LOGE(TAG, "Unable to create socket%d: errno %d", i, errno);
                break;
            }
            //fcntl(sock[i],F_SETFL,O_NONBLOCK);
            ESP_LOGI(TAG, "Socket %d created", i);
        }

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

        if (addr_family == AF_INET)
        {
            for (int i = 0; i < 3; i++)
            {
                struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr[i];
                dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
                dest_addr_ip4->sin_family = AF_INET;
                dest_addr_ip4->sin_port = htons(PORT[i]);
                ip_protocol = IPPROTO_IP;

                int err = bind(sock[i], (struct sockaddr *)&dest_addr[i], sizeof(dest_addr[i]));
                if (err < 0)
                {
                    ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
                }
                ESP_LOGI(TAG, "Socket[%d] bound, port %d", i, PORT[i]);
            }

            // } else if (addr_family == AF_INET6) {
            //     bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
            //     dest_addr.sin6_family = AF_INET6;
            //     dest_addr.sin6_port = htons(PORT);
            //     ip_protocol = IPPROTO_IPV6;
        }

        while (1)
        {

            ESP_LOGI(TAG, "Waiting for data");
            struct sockaddr_storage source_addr[3]; // Large enough for both IPv4 or IPv6

            socklen_t socklen = sizeof(source_addr[0]);
            for (int i = 0; i<2; i ++)
            {
                ESP_LOGI(TAG, "Going to receive for sock[%d]", i);
                int len = recvfrom(sock[i], rx_buffer[i], sizeof(rx_buffer[i]) - 1, 0, (struct sockaddr *)&source_addr[i], &socklen);
                ESP_LOGI(TAG, "recvfrom func done sock[%d]", i);
                // Error occurred during receiving
                if (len < 0)
                {
                    ESP_LOGE(TAG, "recvfrom sock[%d]failed: errno %d",i, errno);
                    continue;
                    //break;
                }
                // Data received
                else
                {
                    // Get the sender's ip address as string
                    if (source_addr[i].ss_family == PF_INET)
                    {
                        inet_ntoa_r(((struct sockaddr_in *)&source_addr[i])->sin_addr, addr_str[i], sizeof(addr_str[i]) - 1);
                    }
                    else if (source_addr[i].ss_family == PF_INET6)
                    {
                        inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr[i])->sin6_addr, addr_str[i], sizeof(addr_str[i]) - 1);
                    }

                    rx_buffer[i][len] = 0; // Null-terminate whatever we received and treat like a string...
                    ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str[i]);
                    ESP_LOGI(TAG, "%s", rx_buffer[i]);

                    int err = sendto(sock[i], rx_buffer[i], len, 0, (struct sockaddr *)&source_addr[i], sizeof(source_addr[i]));
                    if (err < 0)
                    {
                        ESP_LOGE(TAG, "Error occurred during sending to sock[%d]: errno %d",i, errno);
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < 3; i++)
        {
            if (sock[i] != -1)
            {
                ESP_LOGE(TAG, "Shutting down socket and restarting...");
                shutdown(sock[i], 0);
                close(sock[i]);
            }
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

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

#ifdef CONFIG_EXAMPLE_IPV4
    xTaskCreate(udp_server_task, "udp_server", 4096, (void *)AF_INET, 5, NULL);
#endif
#ifdef CONFIG_EXAMPLE_IPV6
    xTaskCreate(udp_server_task, "udp_server", 4096, (void *)AF_INET6, 5, NULL);
#endif
}
