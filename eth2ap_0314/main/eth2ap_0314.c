/* eth2ap (Ethernet to Wi-Fi AP packet forwarding) Example

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

#if CONFIG_ETH_USE_SPI_ETHERNET
#include "driver/spi_master.h"
#endif
#include "freertos/event_groups.h"



static const char *TAG = "eth_example";
static esp_eth_handle_t s_eth_handle = NULL;
static xQueueHandle flow_control_queue = NULL;
static bool s_sta_is_connected = false;
static bool s_ethernet_is_connected = false;
static bool ethernet2wifi_mac_status = false;
static uint8_t s_eth_mac[6];
static esp_netif_t *ap_netif = NULL;
esp_netif_ip_info_t ip_info;

#define FLOW_CONTROL_QUEUE_TIMEOUT_MS (100)
#define FLOW_CONTROL_QUEUE_LENGTH (40)
#define FLOW_CONTROL_WIFI_SEND_TIMEOUT_MS (100)

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group; //only used in station mode

typedef struct
{
    void *packet;
    uint16_t length;
} flow_control_msg_t;

// Forward packets from Wi-Fi to Ethernet
static esp_err_t pkt_wifi2eth(void *buffer, uint16_t len, void *eb)
{
    if (s_ethernet_is_connected)
    {
        //printf("pkt_wifi2eth is here\n\r ");
        if (esp_eth_transmit(s_eth_handle, buffer, len) != ESP_OK)
        {
            ESP_LOGE(TAG, "Ethernet send packet failed");
        }
    }
    // printf("Length: %d Pkt_wifi2eth\n\r",len);
    // for(int i=0;i<len;i++){
    //     printf("%c",((char*)buffer)[i]);
    // }
    esp_wifi_internal_free_rx_buffer(eb);
    return ESP_OK;
}

// Forward packets from Ethernet to Wi-Fi
// Note that, Ethernet works faster than Wi-Fi on ESP32,
// so we need to add an extra queue to balance their speed difference.
static esp_err_t pkt_eth2wifi(esp_eth_handle_t eth_handle, uint8_t *buffer, uint32_t len, void *priv)
{
    esp_err_t ret = ESP_OK;
    flow_control_msg_t msg = {
        .packet = buffer,
        .length = len};
    if (xQueueSend(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) != pdTRUE)
    {
        //printf("eth2wifi is here:%s**********\n\r",buffer);
        ESP_LOGE(TAG, "send flow control message failed or timeout");
        free(buffer);
        ret = ESP_FAIL;
    }
    //printf("pkt_eth2wifi result res=  %d --------", ret);
    return ret;
}

// This task will fetch the packet from the queue, and then send out through Wi-Fi.
// Wi-Fi handles packets slower than Ethernet, we might add some delay between each transmitting.
static void eth2wifi_flow_control_task(void *args)
{
    flow_control_msg_t msg;
    int res = 0;
    uint32_t timeout = 0;
    while (1)
    {
        if (xQueueReceive(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) == pdTRUE)
        {
            timeout = 0;
            // ESP_LOGI(TAG, "Ethernet s_eth_mac Addr %02x:%02x:%02x:%02x:%02x:%02x",
            // s_eth_mac[0], s_eth_mac[1], s_eth_mac[2], s_eth_mac[3], s_eth_mac[4], s_eth_mac[5]);
            if (s_sta_is_connected && msg.length)
            {
                do
                {
                    vTaskDelay(pdMS_TO_TICKS(timeout));
                    timeout += 2;
                    res = esp_wifi_internal_tx(WIFI_IF_AP, msg.packet, msg.length);
                } while (res && timeout < FLOW_CONTROL_WIFI_SEND_TIMEOUT_MS);
                if (res != ESP_OK)
                {
                    ESP_LOGE(TAG, "WiFi send packet failed: %d", res);
                }
            }
            free(msg.packet);
        }
    }
    vTaskDelete(NULL);
}

// Event handler for Ethernet
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    // uint8_t mac_addr[6] = {0};
    // /* we can get the ethernet driver handle from event data */
    // esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id)
    {
    case ETHERNET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Up");
        printf("printf here\n\r");
        s_ethernet_is_connected = true;
        esp_eth_ioctl(s_eth_handle, ETH_CMD_G_MAC_ADDR, s_eth_mac);
        ESP_LOGI(TAG, "Ethernet s_eth_mac Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 s_eth_mac[0], s_eth_mac[1], s_eth_mac[2], s_eth_mac[3], s_eth_mac[4], s_eth_mac[5]);
        esp_wifi_set_mac(WIFI_IF_AP, s_eth_mac); //found! this is where yingshe mac address
        ESP_ERROR_CHECK(esp_wifi_start()); 
        esp_netif_get_ip_info(ap_netif, &ip_info);
        ESP_LOGI(TAG, "THis AP has got an IP as:%s", ip4addr_ntoa((ip4_addr_t*)&ip_info.ip));
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");

        s_ethernet_is_connected = false;
        ethernet2wifi_mac_status =false;
        ESP_ERROR_CHECK(esp_wifi_stop());
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

// Event handler for Wi-Fi
static int s_retry_num = 0;
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    static uint8_t s_con_cnt = 0;
    if(event_id == WIFI_EVENT_AP_STACONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi AP got a station connected");
        // wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *) event_data;
        // ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
        //          MAC2STR(event->mac), event->aid);
        //这里是不开板子的DHCPer服务
        if (!s_con_cnt) {
            s_sta_is_connected = true;
            esp_wifi_internal_reg_rxcb(WIFI_IF_AP, pkt_wifi2eth);
        }
        s_con_cnt++;
    }else if(event_id == WIFI_EVENT_AP_STADISCONNECTED){
        ESP_LOGI(TAG, "Wi-Fi AP got a station disconnected");
        // wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *) event_data;
        // ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                //  MAC2STR(event->mac), event->aid);
        s_con_cnt--;
        if (!s_con_cnt) {
            s_sta_is_connected = false;
            esp_wifi_internal_reg_rxcb(WIFI_IF_AP, NULL);
        }
    }
    //else if(event_id == IP_EVENT_AP_STAIPASSIGNED){
    //     ESP_LOGI(TAG, "Got in IP EVENT");
    //     ip_event_ap_staipassigned_t *event =(ip_event_ap_staipassigned_t *) event_data;
    //     ESP_LOGI(TAG, "A Station has got ip:" IPSTR, IP2STR(&event->ip));
    //     //以下是开板子的DHCP Server
    //     if (!s_con_cnt) {
    //         s_sta_is_connected = true;
    //         esp_wifi_internal_reg_rxcb(WIFI_IF_AP, pkt_wifi2eth);
    //     }
    //     s_con_cnt++;
    // }
}

static void initialize_ethernet(void)
{
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler, NULL));
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO;
#if CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
    mac_config.smi_mdc_gpio_num = CONFIG_EXAMPLE_ETH_MDC_GPIO;
    mac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
#if CONFIG_EXAMPLE_ETH_PHY_IP101
    esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_RTL8201
    esp_eth_phy_t *phy = esp_eth_phy_new_rtl8201(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_LAN8720
    esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_DP83848
    esp_eth_phy_t *phy = esp_eth_phy_new_dp83848(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_KSZ8041
    esp_eth_phy_t *phy = esp_eth_phy_new_ksz8041(&phy_config);
#endif
#elif CONFIG_ETH_USE_SPI_ETHERNET
    gpio_install_isr_service(0);
    spi_device_handle_t spi_handle = NULL;
    spi_bus_config_t buscfg = {
        .miso_io_num = CONFIG_EXAMPLE_ETH_SPI_MISO_GPIO,
        .mosi_io_num = CONFIG_EXAMPLE_ETH_SPI_MOSI_GPIO,
        .sclk_io_num = CONFIG_EXAMPLE_ETH_SPI_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(CONFIG_EXAMPLE_ETH_SPI_HOST, &buscfg, 1));
#if CONFIG_EXAMPLE_USE_DM9051
    spi_device_interface_config_t devcfg = {
        .command_bits = 1,
        .address_bits = 7,
        .mode = 0,
        .clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
        .spics_io_num = CONFIG_EXAMPLE_ETH_SPI_CS_GPIO,
        .queue_size = 20};
    ESP_ERROR_CHECK(spi_bus_add_device(CONFIG_EXAMPLE_ETH_SPI_HOST, &devcfg, &spi_handle));
    /* dm9051 ethernet driver is based on spi driver */
    eth_dm9051_config_t dm9051_config = ETH_DM9051_DEFAULT_CONFIG(spi_handle);
    dm9051_config.int_gpio_num = CONFIG_EXAMPLE_ETH_SPI_INT_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_dm9051(&dm9051_config, &mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_dm9051(&phy_config);
#elif CONFIG_EXAMPLE_USE_W5500
    spi_device_interface_config_t devcfg = {
        .command_bits = 16, // Actually it's the address phase in W5500 SPI frame
        .address_bits = 8,  // Actually it's the control phase in W5500 SPI frame
        .mode = 0,
        .clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
        .spics_io_num = CONFIG_EXAMPLE_ETH_SPI_CS_GPIO,
        .queue_size = 20};
    ESP_ERROR_CHECK(spi_bus_add_device(CONFIG_EXAMPLE_ETH_SPI_HOST, &devcfg, &spi_handle));
    /* w5500 ethernet driver is based on spi driver */
    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(spi_handle);
    w5500_config.int_gpio_num = CONFIG_EXAMPLE_ETH_SPI_INT_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_w5500(&phy_config);
#endif
#endif // CONFIG_ETH_USE_SPI_ETHERNET
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    config.stack_input = pkt_eth2wifi;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &s_eth_handle));

    //----------------------------------//
    // char* ip= "10.13.21.17";
    // char* gateway = "10.13.21.1";
    // char* netmask = "255.255.255.0";
    // ESP_ERROR_CHECK(esp_netif_dhcpc_stop(eth_netif));
    // esp_netif_ip_info_t eth;
    // memset(&eth, 0, sizeof(esp_netif_ip_info_t));
    // eth.ip.addr =esp_ip4addr_aton((const char *)ip);
    // eth.netmask.addr = esp_ip4addr_aton((const char *)netmask);
    // eth.gw.addr=esp_ip4addr_aton((const char *)gateway);
    // esp_netif_set_ip_info(eth_netif,&eth);
    //ESP_ERROR_CHECK(esp_netif_dhcps_start(eth_netif));
    //----------------------------------//


#if !CONFIG_EXAMPLE_USE_INTERNAL_ETHERNET
    /* The SPI Ethernet module might doesn't have a burned factory MAC address, we cat to set it manually.
       02:00:00 is a Locally Administered OUI range so should not be used except when testing on a LAN under your control.
    */
    ESP_ERROR_CHECK(esp_eth_ioctl(s_eth_handle, ETH_CMD_S_MAC_ADDR, (uint8_t[]){0x02, 0x00, 0x00, 0x12, 0x34, 0x56}));
#endif
    esp_eth_ioctl(s_eth_handle, ETH_CMD_S_PROMISCUOUS, (void *)true); //混杂模式（英语：promiscuous mode）是电脑网络中的术语。是指一台机器的网卡能够接收所有经过它的数据流，而不论其目的地址是否是它。
    esp_eth_start(s_eth_handle);
}

static void initialize_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate(); 
    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    //s_wifi_event_group = xEventGroupCreate();  
    esp_netif_t *netif = esp_netif_create_default_wifi_ap();
    ap_netif = netif;
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_give_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_AP_STAIPASSIGNED,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_give_ip));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); //only this defers from softap
    esp_netif_dhcps_stop(ap_netif);
    char* ap_ip= "192.168.4.1";
    char* ap_gateway = "192.168.4.1";
    char* ap_netmask = "255.255.255.0";
    esp_netif_ip_info_t info_t;
    memset(&info_t, 0, sizeof(esp_netif_ip_info_t));
    ip4addr_aton((const char *)ap_ip, &info_t.ip.addr);
    ip4addr_aton((const char *)ap_gateway, &info_t.gw.addr);
    ip4addr_aton((const char *)ap_netmask, &info_t.netmask.addr);
    esp_netif_set_ip_info(netif, &info_t);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = CONFIG_EXAMPLE_WIFI_SSID,
            .ssid_len = strlen(CONFIG_EXAMPLE_WIFI_SSID),
            .password = CONFIG_EXAMPLE_WIFI_PASSWORD,
            .max_connection = CONFIG_EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .channel = CONFIG_EXAMPLE_WIFI_CHANNEL // default: channel 1
        },
    };
    if (strlen(CONFIG_EXAMPLE_WIFI_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));               //default wifi_mode_ap
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config)); //default wifi_if_sta
    //ESP_ERROR_CHECK(esp_netif_dhcps_stop(netif));

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             CONFIG_EXAMPLE_WIFI_SSID, CONFIG_EXAMPLE_WIFI_PASSWORD, CONFIG_EXAMPLE_WIFI_CHANNEL);
}

static esp_err_t initialize_flow_control(void)
{
    flow_control_queue = xQueueCreate(FLOW_CONTROL_QUEUE_LENGTH, sizeof(flow_control_msg_t));
    if (!flow_control_queue)
    {
        ESP_LOGE(TAG, "create flow control queue failed");
        return ESP_FAIL;
    }
    BaseType_t ret = xTaskCreate(eth2wifi_flow_control_task, "flow_ctl", 2048, NULL, (tskIDLE_PRIORITY + 2), NULL);
    if (ret != pdTRUE)
    {
        ESP_LOGE(TAG, "create flow control task failed");
        return ESP_FAIL;
    }
    return ESP_OK;
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(initialize_flow_control());

    initialize_wifi();
    initialize_ethernet();
    
}
