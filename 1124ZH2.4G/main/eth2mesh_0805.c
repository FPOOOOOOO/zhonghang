// Copyright 2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 0805: combine eth2ap into this
// 0827: 规范了N&R发送形式

#include "mdf_common.h"
#include "mwifi.h"
#include "driver/uart.h"

#include "driver/spi_master.h"
#include "driver/spi_slave.h"
#include "esp_spi_flash.h"

// #define MEMORY_DEBUG
#define BUF_SIZE 512

/**
 * @brief 0805
 */
#define FLOW_CONTROL_QUEUE_TIMEOUT_MS (100)
#define FLOW_CONTROL_QUEUE_LENGTH (40)
#define FLOW_CONTROL_WIFI_SEND_TIMEOUT_MS (100)
static bool node_parent_connected = false;
static bool node_child_connected = false;
static bool s_ethernet_is_connected = false;
wifi_sta_list_t wifi_sta_list = {0x0};
mesh_addr_t parent_bssid = {0};
esp_eth_handle_t eth_handle = NULL;
static xQueueHandle flow_control_queue = NULL;
static bool g_root_got_ip = false;
const uint8_t group_id_list[2][6] = {{0x01, 0x00, 0x5e, 0xae, 0xae, 0xae},
                                     {0x01, 0x00, 0x5e, 0xae, 0xae, 0xaf}};
uint8_t Rootaddr[6] = {0xFF, 0x0, 0x0, 0x1, 0x0, 0x0};

static const uint16_t header = 0xA55A;
static uint16_t recv_header = 0x0000;
#define UART (uint8_t)4
#define SPI (uint8_t)7
static uint8_t meshmsgtype = 0;

// SPI para
#define GPIO_MOSI 13 // 12
#define GPIO_MISO 12 // 13
#define GPIO_SCLK 14 // 15
#define GPIO_CS 15   // 14 SPI for ESP
WORD_ALIGNED_ATTR char sendbuf[129] = "hihu";
#define SENDER_HOST HSPI_HOST
#define RCV_HOST HSPI_HOST

typedef struct
{
    void *packet;
    uint16_t length;
} flow_control_msg_t;

static const char *TAG = "eth2mesh";
esp_netif_t *sta_netif;

static void hjypackup(uint8_t type, uint16_t len, uint16_t diy, void *buffer, uint8_t *CRCpackage)
{
    // uint8_t *newpackage = (uint8_t *)malloc(len + 9);
    // bzero(newpackage, len + 9);
    bzero(CRCpackage, len + 7);
    CRCpackage[0] = header >> 8;
    CRCpackage[1] = header;
    // memcpy(newpackage, header, 2); // header
    CRCpackage[2] = type; // type
    CRCpackage[3] = len >> 8;
    CRCpackage[4] = len; // len
    CRCpackage[5] = diy >> 8;
    CRCpackage[6] = diy;                 // len
    memcpy(CRCpackage + 8, buffer, len); // buffer
}

/**
 * @brief uart initialization
 */
static mdf_err_t uart_initialize()
{
    uart_config_t uart_config = {
        .baud_rate = CONFIG_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    MDF_ERROR_ASSERT(uart_param_config(CONFIG_UART_PORT_NUM, &uart_config));
    // MDF_ERROR_ASSERT(uart_set_pin(CONFIG_UART_PORT_NUM, CONFIG_UART_TX_IO, CONFIG_UART_RX_IO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    MDF_ERROR_ASSERT(uart_set_pin(CONFIG_UART_PORT_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    MDF_ERROR_ASSERT(uart_driver_install(CONFIG_UART_PORT_NUM, 2 * BUF_SIZE, 2 * BUF_SIZE, 0, NULL, 0));
    return MDF_OK;
}

static void uart_handle_task(void *arg)
{
    int recv_length = 0;
    mdf_err_t ret = MDF_OK;
    cJSON *json_root = NULL;
    cJSON *json_addr = NULL;
    cJSON *json_group = NULL;
    cJSON *json_data = NULL;
    cJSON *json_dest_addr = NULL;

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)MDF_MALLOC(BUF_SIZE);
    size_t size = MWIFI_PAYLOAD_LEN;
    char *jsonstring = NULL;
    uint8_t dest_addr[MWIFI_ADDR_LEN] = {0};
    mwifi_data_type_t data_type = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};

    MDF_LOGI("Uart handle task is running");

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);

    /* uart initialization */
    MDF_ERROR_ASSERT(uart_initialize());

    while (1)
    {
        memset(data, 0, BUF_SIZE);
        recv_length = uart_read_bytes(CONFIG_UART_PORT_NUM, data, BUF_SIZE, 100 / portTICK_PERIOD_MS);

        if (recv_length <= 0)
        {
            continue;
        }

        //ESP_LOGI("UART Recv data:", "%s", data);

        uint8_t *uart2mesh_data = (uint8_t *)malloc(recv_length + 7);
        bzero(uart2mesh_data, recv_length + 7);
        hjypackup(UART, recv_length, 0, data, uart2mesh_data);

        esp_err_t ret = ESP_OK;
        flow_control_msg_t msg = {
            .packet = uart2mesh_data,
            .length = recv_length + 7};
        if (xQueueSend(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) != pdTRUE)
        {
            // printf("eth2wifi is here:%s**********\n\r",buffer);
            ESP_LOGE(TAG, "send flow control message failed or timeout");
            free(uart2mesh_data);
            ret = ESP_FAIL;
        }

        // json_root = cJSON_Parse((char *)data);
        // MDF_ERROR_CONTINUE(!json_root, "cJSON_Parse, data format error, data: %s", data);

        // /**
        //  * @brief Check if it is a group address. If it is a group address, data_type.group = true.
        //  */
        // json_addr = cJSON_GetObjectItem(json_root, "dest_addr");
        // json_group = cJSON_GetObjectItem(json_root, "group");

        // if (json_addr)
        // {
        //     data_type.group = false;
        //     json_dest_addr = json_addr;
        // }
        // else if (json_group)
        // {
        //     data_type.group = true;
        //     json_dest_addr = json_group;
        // }
        // else
        // {
        //     MDF_LOGW("Address not found");
        //     cJSON_Delete(json_root);
        //     continue;
        // }

        // /**
        //  * @brief  Convert mac from string format to binary
        //  */
        // do
        // {
        //     uint32_t mac_data[MWIFI_ADDR_LEN] = {0};
        //     sscanf(json_dest_addr->valuestring, MACSTR,
        //            mac_data, mac_data + 1, mac_data + 2,
        //            mac_data + 3, mac_data + 4, mac_data + 5);

        //     for (int i = 0; i < MWIFI_ADDR_LEN; i++)
        //     {
        //         dest_addr[i] = mac_data[i];
        //     }
        // } while (0);

        // json_data = cJSON_GetObjectItem(json_root, "data");
        // char *recv_data = cJSON_PrintUnformatted(json_data);

        // size = asprintf(&jsonstring, "{\"src_addr\": \"" MACSTR "\", \"data\": %s}", MAC2STR(sta_mac), recv_data);
        // ret = mwifi_write(Rootaddr, &data_type, jsonstring, size, true);
        // ret = mwifi_write(NULL, &data_type, jsonstring, size, true);
        // MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(ret));

        // FREE_MEM:
        //     MDF_FREE(recv_data);
        //     MDF_FREE(jsonstring);
        //     cJSON_Delete(json_root);
    }

    MDF_LOGI("Uart handle task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

static void spi_task(void *pvParameters)
{
    int n = 0;
    esp_err_t ret;

    // Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .flags = SPICOMMON_BUSFLAG_IOMUX_PINS, // added IOMUX
        .quadwp_io_num = -1,                   // added -1 default
        .quadhd_io_num = -1,
    };

    // Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {
        .mode = 0,
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0,
        .post_setup_cb = NULL,
        .post_trans_cb = NULL};

    // Configuration for the handshake line
    //  gpio_config_t io_conf={
    //      .intr_type=GPIO_INTR_DISABLE,
    //      .mode=GPIO_MODE_OUTPUT,
    //      .pin_bit_mask=(1<<GPIO_HANDSHAKE)
    //  };

    // Configure handshake line as output
    //  gpio_config(&io_conf);
    // Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

    // Initialize SPI slave interface
    // ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, 1);
    ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    ESP_LOGE(TAG, "SPI SLAVE INIT. (%s)", esp_err_to_name(ret));
    assert(ret == ESP_OK);

    // ESP_LOGE(TAG, "I am 1");
    //  WORD_ALIGNED_ATTR char sendbuf[129] = "hihu";
    WORD_ALIGNED_ATTR char recvbuf[129] = "fuckme";
    memset(recvbuf, 0, 33);
    spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t));
    memset(recvbuf, 0x23, 129);
    // ESP_LOGE(TAG, "I am 2");

    while (1)
    {
        // Clear receive buffer, set send buffer to something sane
        // memset(recvbuf, 0xA5, 129);
        // ESP_LOGE(TAG, "I am 3");
        if (meshmsgtype == SPI)
        {
        }
        else
        {
            int res = sprintf(sendbuf, "Htis is from Nonroot, number %04d.", n);
            if (res >= sizeof(sendbuf))
            {
                printf("Data truncated\n");
            }
        }

        // ESP_LOGE(TAG, "I am 4");
        //  Set up a transaction of 128 bytes to send/receive
        t.length = 128 * 8;
        t.tx_buffer = sendbuf;
        t.rx_buffer = recvbuf;
        /* This call enables the SPI slave interface to send/receive to the sendbuf and recvbuf. The transaction is
        initialized by the SPI master, however, so it will not actually happen until the master starts a hardware transaction
        by pulling CS low and pulsing the clock etc. In this specific example, we use the handshake line, pulled up by the
        .post_setup_cb callback that is called as soon as a transaction is ready, to let the master know it is free to transfer
        data.*/

        // ESP_LOGI(TAG, "isReady is: %d \n\r",isReady);

        ret = spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY);
        // ESP_LOGE(TAG, "I am 5");
        ESP_LOGE(TAG, "SPI. (%s)", esp_err_to_name(ret)); //   Equals  spi_slave_queue_trans() + spi_slave_get_trans_results
        // ESP_LOGI(TAG, "isReady is: %d \n\r",isReady);

        // esp_err_t ret;
        // spi_slave_transaction_t *ret_trans;
        // ToDo: check if any spi transfers in flight
        // ret = spi_slave_queue_trans(RCV_HOST, &t, portMAX_DELAY);
        // ESP_LOGE(TAG, "SPI_SLAVE_QUEUE_TRANS. (%s)", esp_err_to_name(ret));
        // vTaskDelay(pdMS_TO_TICKS(100));
        // ret = spi_slave_get_trans_result(RCV_HOST, &ret_trans, portMAX_DELAY);
        // ESP_LOGE(TAG, "SPI_SLAVE_GET_TRANS_RESULT. (%s)", esp_err_to_name(ret));
        // vTaskDelay(pdMS_TO_TICKS(100));
        // assert(ret_trans == &t);
        // return ESP_OK;

        // spi_slave_transmit does not return until the master has done a transmission, so by here we have sent our data and
        // received data from the master. Print it.
        printf(" %d Received: %s\n", n, recvbuf);
        // added 0713 to transfer via wifi
        uint8_t SPIlength = sizeof(recvbuf);
        uint8_t *spi2mesh_data = (uint8_t *)malloc(SPIlength + 7);
        bzero(spi2mesh_data, SPIlength + 7);

        hjypackup(SPI, SPIlength, 0, recvbuf, spi2mesh_data);

        esp_err_t ret = ESP_OK;

        flow_control_msg_t msg = {
            .packet = spi2mesh_data,
            .length = SPIlength + 7};
        if (xQueueSend(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) != pdTRUE)
        {
            // printf("eth2wifi is here:%s**********\n\r",buffer);
            ESP_LOGE(TAG, "send flow control message failed or timeout");
            free(spi2mesh_data);
            ret = ESP_FAIL;
        }

        n++;
        // vTaskDelay(pdMS_TO_TICKS(1000));
        //  vTaskDelay(pdMS_TO_TICKS(2000));
        if (n == 200)
        {
            printf("Already done : %d\n", n);
            n = 0;
        }
    }
}

static void node_read_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    char *data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};

    MDF_LOGI("Node read task is running");

    for (;;)
    {
        if (!mwifi_is_connected() && !(mwifi_is_started() && esp_mesh_is_root()))
        {
            vTaskDelay(500 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);

        /**
         * @brief Pre-allocated memory to data and size must be specified when passing in a level 1 pointer
         */
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        // ret = mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_read", mdf_err_to_name(ret));
        // MDF_LOGI("Node receive, addr: " MACSTR ", size: %d, data: %s", MAC2STR(src_addr), size, data);

        memcpy((uint16_t *)&recv_header, data, 2);
        memcpy((uint8_t *)&meshmsgtype, data + 2, 1);
        recv_header = ntohs(recv_header);
        if (recv_header == 0xA55A)
        {
            //ESP_LOGI(TAG, "RECV_HEADER IS: %x", recv_header);
            uint8_t *mesh_data = (uint8_t *)malloc(size - 7);
            memcpy(mesh_data, data + 8, size - 7);
            if (meshmsgtype == UART)
            {
                printf("UART:%d \n",size-7);
            }
            else if (meshmsgtype == SPI)
            {
                printf("SPI:\n");
            }
            uart_write_bytes(CONFIG_UART_PORT_NUM, mesh_data, size - 7);

            // for (int i = 0; i < len - 11; i++)
            // {
            //     printf("%c", ((char *)wifi_data)[i]);
            // }
            free(mesh_data);
            // printf("\n\r");
        }

        // if (size == 98 || size == 74)
        // {
        //     MDF_LOGI("Got ICMP From WiFi");
        // }
        /* forwoad to eth */
        if (s_ethernet_is_connected)
        {
            if (esp_eth_transmit(eth_handle, data, size) != ESP_OK)
            {
                ESP_LOGE(TAG, "Ethernet send packet failed");
            }
        }

    }

    MDF_LOGW("Node read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}


// static void node_read_task(void *arg)
// {
//     mdf_err_t ret = MDF_OK;
//     char *data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
//     size_t size = MWIFI_PAYLOAD_LEN;
//     mwifi_data_type_t data_type = {0x0};
//     uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};

//     uint8_t *buffer = NULL;
//     size_t buffer_len = 0;

//     MDF_LOGI("Node read task is running");

//     for (;;)
//     {
//         if (!mwifi_is_connected() && !(mwifi_is_started() && esp_mesh_is_root()))
//         {
//             vTaskDelay(500 / portTICK_RATE_MS);
//             continue;
//         }

//         size = MWIFI_PAYLOAD_LEN;
//         memset(data, 0, MWIFI_PAYLOAD_LEN);

//         /**
//          * @brief Pre-allocated memory to data and size must be specified when passing in a level 1 pointer
//          */
//         ret = mwifi_read(src_addr, &data_type, &buffer, &buffer_len, 100 / portTICK_RATE_MS);
//         // ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
//         //  ret = mwifi_read(src_addr, &data_type, data, &size, 100 / portTICK_RATE_MS);
//         //  MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_read", mdf_err_to_name(ret));
//         //  MDF_LOGI("Node receive, addr: " MACSTR ", size: %d, data: %s", MAC2STR(src_addr), size, data);

//         if (ret == MDF_ERR_MWIFI_TIMEOUT || ret == ESP_ERR_MESH_TIMEOUT)
//         {
//             continue;
//         }
//         else if (ret != MDF_OK)
//         {
//             MDF_LOGW("<%s> mwifi_read", mdf_err_to_name(ret));
//             goto FREE_MEM;
//         }

//         memcpy((uint16_t *)&recv_header, buffer, 2);
//         memcpy((uint8_t *)&meshmsgtype, buffer + 2, 1);
//         recv_header = ntohs(recv_header);
//         if (recv_header == 0xA55A)
//         {
//             //ESP_LOGI(TAG, "RECV_HEADER IS: %x", recv_header);
//             uint8_t *mesh_data = (uint8_t *)malloc(buffer_len - 7);
//             memcpy(mesh_data, buffer + 8, buffer_len - 7);
//             if (meshmsgtype == UART)
//             {
//                 printf("UART:\n");
//             }
//             else if (meshmsgtype == SPI)
//             {
//                 printf("SPI:\n");
//                 memcpy(sendbuf, mesh_data, buffer_len - 7);
//             }
//             uart_write_bytes(CONFIG_UART_PORT_NUM, mesh_data, buffer_len - 7);

//             // for (int i = 0; i < len - 11; i++)
//             // {
//             //     printf("%c", ((char *)wifi_data)[i]);
//             // }
//             free(mesh_data);
//             // printf("\n\r");
//         }

//         /* forwoad to eth */
//         if (s_ethernet_is_connected)
//         {
//             if (esp_eth_transmit(eth_handle, buffer, buffer_len) != ESP_OK)
//             {
//                 ESP_LOGE(TAG, "Ethernet send packet failed");
//             }
//         }

//         // if (buffer_len == 98 || buffer_len == 74)
//         // {
//         //     MDF_LOGI("Got ICMP From WiFi");
//         // }
//         // if (buffer_len == 77 && s_ethernet_is_connected)
//         // {
//         //     MDF_LOGI("HBBOOM!");
//         // }

//     FREE_MEM:
//         MDF_FREE(buffer);
//     }

//     MDF_LOGW("Node read task is exit");

//     MDF_FREE(data);
//     vTaskDelete(NULL);
// }

/**
 * @brief printing system information
 */
static void print_system_info_timercb(void *timer)
{
    uint8_t primary = 0;
    wifi_second_chan_t second = 0;
    mesh_addr_t parent_bssid = {0};
    uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
    wifi_sta_list_t wifi_sta_list = {0x0};

    esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);
    esp_wifi_ap_get_sta_list(&wifi_sta_list);
    esp_wifi_get_channel(&primary, &second);
    esp_mesh_get_parent_bssid(&parent_bssid);

    MDF_LOGI("System information, channel: %d, layer: %d, self mac: " MACSTR ", parent bssid: " MACSTR
             ", parent rssi: %d, node num: %d, free heap: %u",
             primary,
             esp_mesh_get_layer(), MAC2STR(sta_mac), MAC2STR(parent_bssid.addr),
             mwifi_get_parent_rssi(), esp_mesh_get_total_node_num(), esp_get_free_heap_size());

    for (int i = 0; i < wifi_sta_list.num; i++)
    {
        MDF_LOGI("Child mac: " MACSTR, MAC2STR(wifi_sta_list.sta[i].mac));
    }

#ifdef MEMORY_DEBUG

    if (!heap_caps_check_integrity_all(true))
    {
        MDF_LOGE("At least one heap is corrupt");
    }

    mdf_mem_print_heap();
    mdf_mem_print_record();
    mdf_mem_print_task();
#endif /**< MEMORY_DEBUG */
}

/**
 * @brief 0805
 */
// Forward packets from Ethernet to Wi-Fi
// Note that, Ethernet works faster than Wi-Fi on ESP32,
// so we need to add an extra queue to balance their speed difference.
static esp_err_t pkt_eth2mesh(esp_eth_handle_t eth_handle, uint8_t *buffer, uint32_t len, void *priv)
{
    esp_err_t ret = ESP_OK;
    flow_control_msg_t msg = {
        .packet = buffer,
        .length = len};
    // if (len){
    //     MDF_LOGI("ETH Downlinking...");
    // }
    // if (len == 98 || len == 74)
    // {
    //     MDF_LOGI("Got ICMP From ETH");
    // }
    // if(len==42){
    //     MDF_LOGI("Got ARP res From ETH");
    // }
    if (xQueueSend(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) != pdTRUE)
    {
        ESP_LOGE(TAG, "send flow control message failed or timeout");
        free(buffer);
        ret = ESP_FAIL;
    }
    return ret;
}

// This task will fetch the packet from the queue, and then send out through Wi-Fi.
// Wi-Fi handles packets slower than Ethernet, we might add some delay between each transmitting.
static void eth2mesh_flow_control_task(void *args)
{
    flow_control_msg_t msg;
    int res = 0;
    uint32_t timeout = 0;
    mwifi_data_type_t data_type = {0};
    data_type.group = false;
    while (1)
    {
        if (xQueueReceive(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) == pdTRUE)
        {
            timeout = 0;
            if ((node_parent_connected || node_child_connected) && msg.length)
            {
                do
                {
                    vTaskDelay(pdMS_TO_TICKS(timeout));
                    timeout += 2;
                    res = mwifi_write(Rootaddr, &data_type, msg.packet, msg.length, true);
                    // res = mwifi_write(NULL, &data_type, msg.packet, msg.length, true);

                } while (res && timeout < FLOW_CONTROL_WIFI_SEND_TIMEOUT_MS);
                // MDF_ERROR_GOTO(res != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(res));
                if (res != MDF_OK)
                {
                    ESP_LOGE(TAG, "Mesh send packet failed: %d", res);
                }
            }
            free(msg.packet);
        }
    }
    vTaskDelete(NULL);
}

/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id)
    {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        s_ethernet_is_connected = true;
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;

    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        s_ethernet_is_connected = false;
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

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case IP_EVENT_ETH_GOT_IP:
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        const esp_netif_ip_info_t *ip_info = &event->ip_info;

        MDF_LOGI("Ethernet Got IP Address");
        MDF_LOGI("ETHIP:" IPSTR, IP2STR(&ip_info->ip));
        MDF_LOGI("ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
        MDF_LOGI("ETHGW:" IPSTR, IP2STR(&ip_info->gw));

        mdf_event_loop_send(MDF_EVENT_MWIFI_ROOT_GOT_IP, NULL);
        g_root_got_ip = true;
        break;
    }

    default:
        break;
    }
}

static mdf_err_t eth_init()
{
    // esp_netif_config_t eth_cfg = ESP_NETIF_DEFAULT_ETH();
    // esp_netif_t *eth_netif = esp_netif_new(&eth_cfg);
    // MDF_ERROR_ASSERT(esp_eth_set_default_handlers(eth_netif));

    MDF_ERROR_ASSERT(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    // MDF_ERROR_ASSERT(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL));

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO;

    mac_config.smi_mdc_gpio_num = CONFIG_EXAMPLE_ETH_MDC_GPIO;
    mac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
    // #if CONFIG_EXAMPLE_ETH_PHY_IP101
    //     esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);
    // #elif CONFIG_EXAMPLE_ETH_PHY_RTL8201
    //     esp_eth_phy_t *phy = esp_eth_phy_new_rtl8201(&phy_config);
    // #elif CONFIG_EXAMPLE_ETH_PHY_LAN8720
    //     esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);
    // #elif CONFIG_EXAMPLE_ETH_PHY_DP83848
    //     esp_eth_phy_t *phy = esp_eth_phy_new_dp83848(&phy_config);
    // #endif

    // esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_rtl8201(&phy_config);
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    config.stack_input = pkt_eth2mesh;
    MDF_ERROR_ASSERT(esp_eth_driver_install(&config, &eth_handle));
    // MDF_ERROR_ASSERT(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    esp_eth_ioctl(eth_handle, ETH_CMD_S_PROMISCUOUS, (void *)true);
    MDF_ERROR_ASSERT(esp_eth_start(eth_handle));

    return MDF_OK;
}

static mdf_err_t wifi_init()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_netif_create_default_wifi_mesh_netifs(&sta_netif, NULL));
    MDF_ERROR_ASSERT(esp_wifi_init(&cfg));
    MDF_ERROR_ASSERT(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    MDF_ERROR_ASSERT(esp_wifi_set_mode(WIFI_MODE_STA));
    MDF_ERROR_ASSERT(esp_wifi_set_ps(WIFI_PS_NONE));
    MDF_ERROR_ASSERT(esp_mesh_set_6m_rate(false));
    MDF_ERROR_ASSERT(esp_wifi_start());

    return MDF_OK;
}

/**
 * @brief All module events will be sent to this task in esp-mdf
 *
 * @Note:
 *     1. Do not block or lengthy operations in the callback function.
 *     2. Do not consume a lot of memory in the callback function.
 *        The task memory of the callback function is only 4KB.
 */
static mdf_err_t event_loop_cb(mdf_event_loop_t event, void *ctx)
{
    MDF_LOGI("event_loop_cb, event: %d", event);

    switch (event)
    {
    case MDF_EVENT_MWIFI_STARTED:
        MDF_LOGI("MESH is started");
        break;

    case MDF_EVENT_MWIFI_PARENT_CONNECTED:
        MDF_LOGI("Parent is connected on station interface");
        node_parent_connected = true;
        esp_mesh_get_parent_bssid(&parent_bssid);
        if (esp_mesh_is_root())
        {
            esp_netif_dhcpc_start(sta_netif);
        }

        break;

    case MDF_EVENT_MWIFI_PARENT_DISCONNECTED:
        MDF_LOGI("Parent is disconnected on station interface");
        node_parent_connected = false;
        break;

    case MDF_EVENT_MWIFI_CHILD_CONNECTED:
        MDF_LOGI("Child is connected on ap interface");
        node_child_connected = true;
        esp_wifi_ap_get_sta_list(&wifi_sta_list);
        break;

    case MDF_EVENT_MWIFI_CHILD_DISCONNECTED:
        MDF_LOGI("Child is disconnected on ap interface");
        node_child_connected = false;
        break;

    default:
        break;
    }

    return MDF_OK;
}

static esp_err_t initialize_flow_control(void)
{
    flow_control_queue = xQueueCreate(FLOW_CONTROL_QUEUE_LENGTH, sizeof(flow_control_msg_t));
    if (!flow_control_queue)
    {
        ESP_LOGE(TAG, "create flow control queue failed");
        return ESP_FAIL;
    }
    xTaskCreatePinnedToCore(eth2mesh_flow_control_task, "flow_ctl", 4 * 1024,
                            NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY,
                            NULL, CONFIG_MDF_TASK_PINNED_TO_CORE);
    // BaseType_t ret = xTaskCreate(eth2mesh_flow_control_task, "flow_ctl", 2048, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    // if (ret != pdTRUE)
    // {
    //     ESP_LOGE(TAG, "create flow control task failed");
    //     return ESP_FAIL;
    // }
    return ESP_OK;
}

void app_main()
{
    mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT();
    mwifi_config_t config = {
        .channel = CONFIG_MESH_CHANNEL,
        .mesh_id = CONFIG_MESH_ID,
        .mesh_type = CONFIG_DEVICE_TYPE,
    };

    /**
     * @brief Set the log level for serial port printing.
     */
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    /**
     * @brief Initialize wifi mesh.
     */
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb));
    mdf_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        MDF_ERROR_ASSERT(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    MDF_ERROR_ASSERT(ret);

    MDF_ERROR_ASSERT(esp_netif_init());
    MDF_ERROR_ASSERT(esp_event_loop_create_default());
    ESP_ERROR_CHECK(initialize_flow_control());
    MDF_ERROR_ASSERT(eth_init());
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mwifi_init(&cfg));
    MDF_ERROR_ASSERT(mwifi_set_config(&config));
    MDF_ERROR_ASSERT(mwifi_start());

    /**
     * @brief select/extend a group memebership here
     *      group id can be a custom address
     */

    MDF_ERROR_ASSERT(esp_mesh_set_group_id((mesh_addr_t *)group_id_list,
                                           sizeof(group_id_list) / sizeof(group_id_list[0])));

    /**
     * @brief Data transfer between wifi mesh devices
     */
    xTaskCreatePinnedToCore(node_read_task, "node_read_task", 4 * 1024,
                            NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY,
                            NULL, CONFIG_MDF_TASK_PINNED_TO_CORE);
    // xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
    //             NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

    /* Periodic print system information */
    // TimerHandle_t timer = xTimerCreate("print_system_info", 10000 / portTICK_RATE_MS,
    //                                    true, NULL, print_system_info_timercb);
    // xTimerStart(timer, 0);

    /**
     * @brief uart handle task:
     *  receive json format data,eg:`{"dest_addr":"30:ae:a4:80:4c:3c","data":"send data"}`
     *  forward data item to destination address in mesh network
     */
    xTaskCreate(uart_handle_task, "uart_handle_task", 4 * 1024,
                NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

    //xTaskCreate(spi_task, "spi_task", 4096, NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY + 6, NULL);
}
