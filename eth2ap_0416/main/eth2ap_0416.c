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

#include "driver/uart.h"
#include <sys/param.h>
#include "lwip/sockets.h"
#include <lwip/netdb.h>
#include "/Users/fpo/esp/esp-idf/examples/common_components/protocol_examples_common/include/addr_from_stdin.h"

PRIVILEGED_DATA static portMUX_TYPE xTaskQueueMutex = portMUX_INITIALIZER_UNLOCKED;

static const char *TAG = "eth2ap";
static const char *payload = "AThis is msg from Server";
static esp_eth_handle_t s_eth_handle = NULL;
static xQueueHandle flow_control_queue = NULL;
static bool s_sta_is_connected = false;
static bool s_ethernet_is_connected = false;
static bool ethernet2wifi_mac_status = false;
static uint8_t s_eth_mac[6];
static esp_netif_t *ap_netif = NULL;
esp_netif_ip_info_t ip_info;
esp_ip4_addr_t stationip[16];
uint16_t port[16];
char read_buff[1500];      //接收数据的缓存数组
uint8_t frame_buffer[200]; //测试80211tx
// package
static const uint16_t header = 0xA55A;
#define UART (uint8_t)4
#define SPI (uint8_t)7
//#define STA1 0x00000001
static uint16_t recv_header = 0x0000;
static uint8_t whatinit = 0x00;

#define FLOW_CONTROL_QUEUE_TIMEOUT_MS (100)
#define FLOW_CONTROL_QUEUE_LENGTH (40)
#define FLOW_CONTROL_WIFI_SEND_TIMEOUT_MS (100)
// UART Para
#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM (3) /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define UART_BUFSIZE (1024)
//#define RD_BUF_SIZE (BUF_SIZE)
static QueueHandle_t uart0_queue;
// UART end

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define PORT CONFIG_EXAMPLE_PORT

typedef struct
{
    void *packet;
    uint16_t length;
} flow_control_msg_t;

typedef struct
{
    uint16_t header;
    uint8_t type;
    uint32_t dest;
    uint8_t source;
    uint16_t len;
    void *buffer;
    uint16_t CRC;
} package_msg_t;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group; // only used in station mode

// CRC begin
void InvertUint8(unsigned char *dBuf, unsigned char *srcBuf)
{
    int i;
    unsigned char tmp[4];
    tmp[0] = 0;
    for (i = 0; i < 8; i++)
    {
        if (srcBuf[0] & (1 << i))
            tmp[0] |= 1 << (7 - i);
    }
    dBuf[0] = tmp[0];
}

void InvertUint16(unsigned short *dBuf, unsigned short *srcBuf)
{
    int i;
    unsigned short tmp[4];
    tmp[0] = 0;
    for (i = 0; i < 16; i++)
    {
        if (srcBuf[0] & (1 << i))
            tmp[0] |= 1 << (15 - i);
    }
    dBuf[0] = tmp[0];
}

uint16_t CRC16_MODBUS(uint8_t *puchMsg, uint16_t usDataLen)
{
    uint16_t wCRCin = 0xFFFF;
    uint16_t wCPoly = 0x8005;
    uint8_t wChar = 0;

    while (usDataLen--)
    {
        wChar = *(puchMsg++);
        InvertUint8(&wChar, &wChar);
        wCRCin ^= (wChar << 8);
        for (int i = 0; i < 8; i++)
        {
            if (wCRCin & 0x8000)
                wCRCin = (wCRCin << 1) ^ wCPoly;
            else
                wCRCin = wCRCin << 1;
        }
    }
    InvertUint16(&wCRCin, &wCRCin);
    return (wCRCin);
}
// CRC end

static void hjypackup(uint8_t type, uint32_t dest, uint8_t source, uint16_t len, void *buffer, uint8_t *CRCpackage)
{
    // uint8_t *newpackage = (uint8_t *)malloc(len + 9);
    // bzero(newpackage, len + 9);
    bzero(CRCpackage, len + 11);
    CRCpackage[0] = header >> 8;
    CRCpackage[1] = header;
    // memcpy(newpackage, header, 2); // header
    CRCpackage[2] = type; // type
    CRCpackage[3] = dest >> 16;
    CRCpackage[4] = dest >> 8;
    CRCpackage[5] = dest;   // dest
    CRCpackage[6] = source; // source
    CRCpackage[7] = len >> 8;
    CRCpackage[8] = len;                  // len
    memcpy(CRCpackage + 10, buffer, len); // buffer
    // uint8_t *CRCpackage = (uint8_t *)malloc(len+11);
    // memcpy(CRCpackage, newpackage, len + 9);
    // CRCpackage[len + 9] = CRC16_MODBUS(newpackage, len + 9);
    // free(newpackage);
}

// static void hjyunpack(void *buffer,package_msg_t pkg){

// }


// Forward packets from Wi-Fi to Ethernet
static esp_err_t pkt_wifi2eth(void *buffer, uint16_t len, void *eb)
{
    ESP_LOGI(TAG, "LEN =: %d", len);
    if (len > 60)
    {
        memcpy((esp_ip4_addr_t *)&stationip[0], buffer + 26, 4); // source ip
        memcpy((uint16_t *)&port[0], buffer + 36, 2);            // source port
        memcpy(read_buff, buffer + 42, len - 42);                // source data
        ESP_LOGI(TAG, "Recv socket by WIFI: %s", read_buff);
        ESP_LOGI(TAG, "This message is from:%s", ip4addr_ntoa((ip4_addr_t *)&stationip[0]));
        ESP_LOGI(TAG, "Port is:%d", ntohs(port[0]));
    }

    // memcpy((uint16_t *)&recv_header, buffer, 2);
    // recv_header = ntohs(recv_header);
    // ESP_LOGI(TAG, "RECV_HEADER IS: %x", recv_header);
    // if (recv_header == 0xA55A)
    // {
    //     uint8_t *wifi_data = (uint8_t *)malloc(len - 11);
    //     memcpy(wifi_data, buffer + 9, len - 11);
    //     //     recv_data.header=recv_header;
    //     //     memcpy((uint8_t *)&(recv_data.type),buffer+2,1);
    //     //     memcpy((uint32_t *)&(recv_data.dest),buffer+3,3);
    //     //     memcpy((uint8_t *)&(recv_data.source),buffer+6,1);
    //     //     memcpy((uint16_t *)&(recv_data.len),buffer+7,2);
    //     //     memcpy(wifi_data,buffer+9,len-11);
    //     //     recv_data.buffer = wifi_data;
    //     //     memcpy((uint16_t *)&(recv_data.CRC),buffer+len-2,2);
    //     //     if(recv_data.type==UART){
    //     //         uart_write_bytes(EX_UART_NUM, (const char *)recv_data.buffer, recv_data.len-11);
    //     //     }
    //     uart_write_bytes(EX_UART_NUM, (const char *)wifi_data, len-11);

    //     // for (int i = 0; i < len - 11; i++)
    //     // {
    //     //     printf("%c", ((char *)wifi_data)[i]);
    //     // }
    //     free(wifi_data);
    //     // printf("\n\r");
    // }

    if (s_ethernet_is_connected)
    {
        // printf("pkt_wifi2eth is here\n\r ");
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
        // printf("eth2wifi is here:%s**********\n\r",buffer);
        ESP_LOGE(TAG, "send flow control message failed or timeout");
        free(buffer);
        ret = ESP_FAIL;
    }
    // printf("pkt_eth2wifi result res=  %d --------", ret);
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
            // if (!ethernet2wifi_mac_status)
            // {
            //     memcpy(s_eth_mac, (uint8_t *)msg.packet + 6, sizeof(s_eth_mac)); // added from eth2wifi
            //     esp_wifi_set_mac(WIFI_IF_AP, s_eth_mac);                         // reset the sta eth once more.added
            //     ethernet2wifi_mac_status = true;
            // }

            if (s_sta_is_connected && msg.length)
            {
                printf("I am bad guys \n\r");
                do
                {
                    vTaskDelay(pdMS_TO_TICKS(timeout));
                    timeout += 2;
                    memcpy(&whatinit, msg.packet, 1);
                    printf("What's in it:%x", whatinit);
                    res = esp_wifi_internal_tx(WIFI_IF_AP, msg.packet, msg.length);
                    // res = esp_wifi_80211_tx(WIFI_IF_AP, msg.packet, msg.length,true);
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

static void uart_task(void *pvParameters)
{
    //先初始化UART
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // Install UART driver, and get the queue.
    uart_driver_install(EX_UART_NUM, UART_BUFSIZE * 2, UART_BUFSIZE * 2, 20, &uart0_queue, 0);
    uart_param_config(EX_UART_NUM, &uart_config);

    // Set UART log level
    esp_log_level_set(TAG, ESP_LOG_INFO);
    // Set UART pins (using UART0 default pins ie no changes.)
    uart_set_pin(EX_UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Set uart pattern detect function.
    uart_enable_pattern_det_baud_intr(EX_UART_NUM, '+', PATTERN_CHR_NUM, 9, 0, 0);
    // Reset the pattern queue length to record at most 20 pattern positions.
    uart_pattern_queue_reset(EX_UART_NUM, 20);

    uart_event_t event;
    size_t buffered_size;
    uint8_t *uart_recv_data = (uint8_t *)malloc(UART_BUFSIZE);

    for (;;)
    {
        // Waiting for UART event.
        if (xQueueReceive(uart0_queue, (void *)&event, (portTickType)portMAX_DELAY))
        {
            bzero(uart_recv_data, UART_BUFSIZE);
            ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
            switch (event.type)
            {
            // Event of UART receving data
            /*We'd better handler data event fast, there would be much more data events than
            other types of events. If we take too much time on data event, the queue might
            be full.*/
            case UART_DATA:
                // portENTER_CRITICAL(&xTaskQueueMutex);//进入临界点
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(EX_UART_NUM, uart_recv_data, event.size, portMAX_DELAY);
                ESP_LOGI(TAG, "[DATA EVT]:");
                // bzero(frame_buffer, 200);
                // // First 4 bytes bytes of the frame. The rest of the frame might be Cisco proprietary and thus fall under NDA.
                // frame_buffer[0] = 0x08; // Version (B0-1): 00 (802.11), Type (B2-3): 10 (Data) Subtype: 0000 (Data) _ 0000100 = 0x08
                // frame_buffer[1] = 0x02; // To DS: 0 From DS: 1 More Fragments: 0 Retry: 0, Power Management: 0 More Data: 0, Protected Frame_buffer: 0, +HTC Order: 0
                // // 2-3 Duration - The Duration field must be set to all zeros.
                // frame_buffer[2] = 0x00;
                // frame_buffer[3] = 0x00;
                // // 4-9 Destination address - Multicast address (1)
                // // 16-21 BSSID - Unused (3), set to 0.0.0.0
                // // 24-29 Source address - Unused (4), set to 0.0.0.0
                // frame_buffer[4] = 0x24;
                // frame_buffer[5] = 0xd7;
                // frame_buffer[6] = 0xeb;
                // frame_buffer[7] = 0xbb;
                // frame_buffer[8] = 0x9a;
                // frame_buffer[9] = 0x6c;
                // frame_buffer[10] = 0x8c;
                // frame_buffer[11] = 0x4b;
                // frame_buffer[12] = 0x14;
                // frame_buffer[13] = 0x14;
                // frame_buffer[14] = 0x77;
                // frame_buffer[15] = 0xbb;
                // memcpy(frame_buffer+24,uart_recv_data,event.size+24);

                uint8_t *uart2wifi_data = (uint8_t *)malloc(event.size + 11);
                // bzero(uart2wifi_data, event.size + 11);
                hjypackup(UART, 0x0000000000000001, 0x00, event.size, uart_recv_data, uart2wifi_data);
                // uart2wifi_data[0]=header>>8;
                // uart2wifi_data[1]=header;
                esp_err_t ret = ESP_OK;
                flow_control_msg_t msg = {
                    .packet = uart2wifi_data,
                    .length = event.size + 11};
                if (xQueueSend(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) != pdTRUE)
                {
                    // printf("eth2wifi is here:%s**********\n\r",buffer);
                    ESP_LOGE(TAG, "send flow control message failed or timeout");
                    free(uart2wifi_data);
                    ret = ESP_FAIL;
                }

                // int ret = esp_wifi_internal_tx(WIFI_IF_AP, uart2wifi_data, event.size + 11);
                // ESP_LOGE(TAG, "Send wifi data. (%s)", esp_err_to_name(ret));
                // free(uart2wifi_data);

                // for (uint8_t iiii = 1; iiii < 100; iiii = iiii + 1)
                // {
                //     bzero(uart2wifi_data, event.size + 11);
                //     memcpy(uart2wifi_data, &iiii, 1);
                //     memcpy(uart2wifi_data + 4, uart_recv_data, event.size);
                //     // hjypackup(UART,0x0000000000000001,0x00,event.size,uart_recv_data,uart2wifi_data);
                //     int ret = esp_wifi_internal_tx(WIFI_IF_AP, uart2wifi_data, event.size + 11);
                //     ESP_LOGE(TAG, "Send wifi %d data. (%s)", iiii, esp_err_to_name(ret));
                //     vTaskDelay(5 / portTICK_PERIOD_MS);

                // }

                // esp_wifi_80211_tx(WIFI_IF_AP, frame_buffer, sizeof(frame_buffer), true);

                uart_write_bytes(EX_UART_NUM, (const char *)uart_recv_data, event.size);
                // portEXIT_CRITICAL(&xTaskQueueMutex);//退出临界点
                break;
            // Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart0_queue);
                break;
            // Event of UART ring buffer full
            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider encreasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(EX_UART_NUM);
                xQueueReset(uart0_queue);
                break;
            // Event of UART RX break detected
            case UART_BREAK:
                ESP_LOGI(TAG, "uart rx break");
                break;
            // Event of UART parity check error
            case UART_PARITY_ERR:
                ESP_LOGI(TAG, "uart parity error");
                break;
            // Event of UART frame error
            case UART_FRAME_ERR:
                ESP_LOGI(TAG, "uart frame error");
                break;
            // UART_PATTERN_DET
            case UART_PATTERN_DET:
                uart_get_buffered_data_len(EX_UART_NUM, &buffered_size);
                int pos = uart_pattern_pop_pos(EX_UART_NUM);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1)
                {
                    // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
                    // record the position. We should set a larger queue size.
                    // As an example, we directly flush the rx buffer here.
                    uart_flush_input(EX_UART_NUM);
                }
                else
                {
                    uart_read_bytes(EX_UART_NUM, uart_recv_data, pos, 100 / portTICK_PERIOD_MS);
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(EX_UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "read data: %s", uart_recv_data);
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }
                break;
            // Others
            default:
                ESP_LOGI(TAG, "uart event type: %d", event.type);
                break;
            }
        }
    }
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
        s_ethernet_is_connected = true;
        esp_eth_ioctl(s_eth_handle, ETH_CMD_G_MAC_ADDR, s_eth_mac);
        ESP_LOGI(TAG, "Ethernet s_eth_mac Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 s_eth_mac[0], s_eth_mac[1], s_eth_mac[2], s_eth_mac[3], s_eth_mac[4], s_eth_mac[5]);
        ESP_ERROR_CHECK(esp_wifi_set_mac(WIFI_IF_AP, s_eth_mac)); // found! this is where yingshe mac address
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_LOGI(TAG, "ETHERNET_EVENT_CONNECTED DONE\n\r");
        // esp_netif_get_ip_info(ap_netif, &ip_info);
        // ESP_LOGI(TAG, "THis AP has got an IP as:%s", ip4addr_ntoa((ip4_addr_t*)&ip_info.ip));
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");

        s_ethernet_is_connected = false;
        ethernet2wifi_mac_status = false;
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
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        ESP_LOGI(TAG, "Wi-Fi AP got a station connected");
        // wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        // ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
        //          MAC2STR(event->mac), event->aid);
        //这里是不开板子的DHCPer服务
        if (!s_con_cnt)
        {
            s_sta_is_connected = true;
            esp_wifi_set_promiscuous_rx_cb(pkt_wifi2eth);
            esp_wifi_internal_reg_rxcb(WIFI_IF_AP, pkt_wifi2eth);
        }
        s_con_cnt++;
        ESP_LOGI(TAG, "Afer joining,now has %d STA connection", s_con_cnt);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        ESP_LOGI(TAG, "Wi-Fi AP got a station disconnected");
        // wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        // ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
        //          MAC2STR(event->mac), event->aid);
        s_con_cnt--;
        ESP_LOGI(TAG, "After leaving,now has %d STA connection", s_con_cnt);
        if (!s_con_cnt)
        {
            s_sta_is_connected = false;
            esp_wifi_internal_reg_rxcb(WIFI_IF_AP, NULL);
        }
    }
    // else if (event_id == IP_EVENT_AP_STAIPASSIGNED)
    // {
    //     ESP_LOGI(TAG, "Got in IP EVENT");
    //     ip_event_ap_staipassigned_t *event = (ip_event_ap_staipassigned_t *)event_data;
    //     ESP_LOGI(TAG, "A Station has got ip:" IPSTR, IP2STR(&event->ip));
    //     //以下是开板子的DHCP Server
    //     //  if (!s_con_cnt) {
    //     //      s_sta_is_connected = true;
    //     //      esp_wifi_internal_reg_rxcb(WIFI_IF_AP, pkt_wifi2eth);
    //     //  }
    //     //  s_con_cnt++;
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
    // ESP_ERROR_CHECK(esp_netif_dhcps_start(eth_netif));
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
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    // s_wifi_event_group = xEventGroupCreate();
    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    // s_wifi_event_group = xEventGroupCreate();
    ap_netif = esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); // only this defers from softap

    // esp_event_handler_instance_t instance_any_id;
    // esp_event_handler_instance_t instance_give_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

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
    if (strlen(CONFIG_EXAMPLE_WIFI_PASSWORD) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));               // default wifi_mode_ap
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config)); // default wifi_if_sta
    // ESP_ERROR_CHECK(esp_wifi_start());

    int ret = esp_netif_dhcps_stop(ap_netif);
    printf("stop dhcp ret = %x\n", ret);
    char *ap_ip = "192.168.4.2";
    char *ap_gateway = "192.168.4.1";
    char *ap_netmask = "255.255.255.0";
    esp_netif_ip_info_t info_t;
    memset(&info_t, 0, sizeof(esp_netif_ip_info_t));
    ip4addr_aton((const char *)ap_ip, &info_t.ip.addr);
    ip4addr_aton((const char *)ap_gateway, &info_t.gw.addr);
    ip4addr_aton((const char *)ap_netmask, &info_t.netmask.addr);
    esp_netif_set_ip_info(ap_netif, &info_t);

    esp_err_t rett = esp_netif_get_ip_info(ap_netif, &ip_info);
    printf("get_ip_info ret = %u\n", rett);

    ESP_LOGI(TAG, "This AP's ip is %s.", ip4addr_ntoa((ip4_addr_t *)&info_t.ip));
    ESP_LOGI(TAG, "HAS GOT HERE");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    // ESP_ERROR_CHECK(esp_netif_dhcps_stop(ap_netif));

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

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(initialize_flow_control());

    initialize_wifi();
    initialize_ethernet();

    //  Create a task to handler UART event from ISR
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 12, NULL);
    // xTaskCreate(udp_server_task, "udp_server_task", 4096, NULL, 5, NULL);
    //  initialize_udp_server();
}
