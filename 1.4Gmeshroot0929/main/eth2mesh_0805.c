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

#include <stdio.h>
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <sys/select.h>

#include "esp_vfs.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"

#include "esp_private/wifi.h"

#include <stdlib.h>

#include "sdkconfig.h"
#include "esp_eth.h"

#define GPIO_OUTPUT_IO_0 25 // SPI?_CLK（IO32） SPI_MOSI（IO2）  SPINSS(IO16)
#define GPIO_OUTPUT_IO_1 16 // NSS
#define GPIO_OUTPUT_IO_2 2  // MOSI
#define GPIO_OUTPUT_IO_3 32 // CLK
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1) | (1ULL << GPIO_OUTPUT_IO_2) | (1ULL << GPIO_OUTPUT_IO_3))
#define GPIO_INPUT_IO_0 34
#define GPIO_INPUT_IO_1 5
#define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

#define ADF_CE_Set (gpio_set_level(GPIO_OUTPUT_IO_0, 1))
#define ADF_CE_Clr (gpio_set_level(GPIO_OUTPUT_IO_0, 0))

#define ADF_LE_Set (gpio_set_level(GPIO_OUTPUT_IO_1, 1))
#define ADF_LE_Clr (gpio_set_level(GPIO_OUTPUT_IO_1, 0))

#define ADF_DATA_Set (gpio_set_level(GPIO_OUTPUT_IO_2, 1))
#define ADF_DATA_Clr (gpio_set_level(GPIO_OUTPUT_IO_2, 0))

#define ADF_CLK_Set (gpio_set_level(GPIO_OUTPUT_IO_3, 1))
#define ADF_CLK_Clr (gpio_set_level(GPIO_OUTPUT_IO_3, 0))

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
uint8_t Multiaddr[6] = {0};
WORD_ALIGNED_ATTR char test14G[77] = "hihu";

static xQueueHandle gpio_evt_queue = NULL;
uint32_t R = 100;   // R为参考分配器的数值，计算公式：输入频率/（2*R）=0.1
                    // 注：输入频率的单位为MHz，R的范围为0~1023的整数
                    // 默认输入频率为板载25M晶振，故得R为125。
                    // 板载40M晶振，则R=200
uint32_t F = 38000; // 350 初始频率35MHz

typedef struct
{
    void *packet;
    uint16_t length;
} flow_control_msg_t;

static const char *TAG = "eth2mesh";
esp_netif_t *sta_netif;

static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void *arg)
{
    uint32_t io_num;
    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
        }
    }
}

static void GPIO_INIT(void)
{
    // zero-initialize the config structure.
    gpio_config_t io_conf = {};
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);

    // // interrupt of rising edge
    // io_conf.intr_type = GPIO_INTR_POSEDGE;
    // // bit mask of the pins, use GPIO4/5 here
    // io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    // // set as input mode
    // io_conf.mode = GPIO_MODE_INPUT;
    // // enable pull-up mode
    // io_conf.pull_up_en = 1;
    // gpio_config(&io_conf);

    // // change gpio intrrupt type for one pin
    // gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    // create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    // start gpio task
    // xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    // install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    // // hook isr handler for specific gpio pin
    // gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void *)GPIO_INPUT_IO_0);
    // // hook isr handler for specific gpio pin
    // gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void *)GPIO_INPUT_IO_1);

    // // remove isr handler for gpio number.
    // gpio_isr_handler_remove(GPIO_INPUT_IO_0);
    // // hook isr handler for specific gpio pin again
    // gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void *)GPIO_INPUT_IO_0);

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());
}

//-----------------------------------------------------------------
//函数名称:void ADF4351_Wdata(uint32_t dat)
//函数功能:ADF4351写数据
//入口参数:无
//出口参数:无
//-----------------------------------------------------------------

void ADF4351_Wdata(uint32_t dat)
{
    uint8_t i;
    ADF_CLK_Clr;
    ADF_LE_Clr; //让32位移位寄存器准备接受下一次数据
    for (i = 0; i < 32; i++)
    {
        if (dat & 0x80000000)
            ADF_DATA_Set;
        else
            ADF_DATA_Clr;
        dat <<= 1;
        ADF_CLK_Set;
        ADF_CLK_Clr; //数据在CLK上升沿时逐个输入32位移位寄存器
    }
    ADF_LE_Set; //打高使得32位移位寄存器按?的寄存器地址进行写入
}

//-----------------------------------------------------------------
//函数名称:void ADF4351_Init(uint32_t date)
//函数功能:ADF4351初始化
//入口参数:无
//出口参数:无
//-----------------------------------------------------------------

void ADF4351_Init(uint32_t date)
{
    ADF_CE_Set;
    usleep(5);
    // Delay_1us (5);
    // vTaskDelay(pdMS_TO_TICKS(1000));
    // portTICK_PERIOD_MS
    ADF_CLK_Clr;
    ADF_LE_Set;
    ADF_DATA_Clr;
    ADF4351_Wdata(0x00580005);              // 设置寄存器5 ：LD设置为数字锁定监测模式
    ADF4351_Wdata(0x0060a43c);              // 设置寄存器4 : 差分输出功率设置为5dbm，使能静音至检测到锁定，频段选择时钟设为10K
    ADF4351_Wdata(0x006004b3);              // 设置寄存器3	：charge cancellation，ABP设为1
    ADF4351_Wdata(0x0D003Fc2 | R << 14);    // 设置寄存器2：电荷泵电流设为5mA，LDP,LDF设为1。
    ADF4351_Wdata(0x08008011);              //	设置寄存器1 ：预分配器设为8/9
    ADF4351_Wdata(0x00000000 | date << 15); // N分配器数据写入寄存器R0
}

void SetFreq(uint32_t F)
{
    ESP_LOGI(TAG, "I am here3");
    if (F >= 690 && F <= 1370)
        ADF4351_Wdata(0x0050443c); // if,else if中得语句是判断此时频率得范围
    else if (F > 1370 && F <= 2740)
        ADF4351_Wdata(0x0040443c); // 根据频率的范围确定我们要更新此时寄存器4（RF diver）的值
    else if (F > 2740 && F <= 5490)
        ADF4351_Wdata(0x0030143c);
    else if (F > 5490 && F <= 10990)
        ADF4351_Wdata(0x0020143c);
    else if (F > 10990 && F <= 21990)
        ADF4351_Wdata(0x0010143c);
    else if (F > 21990)
        ADF4351_Wdata(0x0000143c);
    else
        ADF4351_Wdata(0x0060443c);

    ADF4351_Wdata(0x00000000 | F << 15); // 把要输出频率的字写入寄存器0，改变输出频率
    ESP_LOGI(TAG, "Now Frequency is %d M:", F);
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

        ESP_LOGI("UART Recv data:", "%s", data);

        json_root = cJSON_Parse((char *)data);
        MDF_ERROR_CONTINUE(!json_root, "cJSON_Parse, data format error, data: %s", data);

        /**
         * @brief Check if it is a group address. If it is a group address, data_type.group = true.
         */
        json_addr = cJSON_GetObjectItem(json_root, "dest_addr");
        json_group = cJSON_GetObjectItem(json_root, "group");

        if (json_addr)
        {
            data_type.group = false;
            json_dest_addr = json_addr;
        }
        else if (json_group)
        {
            data_type.group = true;
            json_dest_addr = json_group;
        }
        else
        {
            MDF_LOGW("Address not found");
            cJSON_Delete(json_root);
            continue;
        }

        /**
         * @brief  Convert mac from string format to binary
         */
        do
        {
            uint32_t mac_data[MWIFI_ADDR_LEN] = {0};
            sscanf(json_dest_addr->valuestring, MACSTR,
                   mac_data, mac_data + 1, mac_data + 2,
                   mac_data + 3, mac_data + 4, mac_data + 5);

            for (int i = 0; i < MWIFI_ADDR_LEN; i++)
            {
                dest_addr[i] = mac_data[i];
                ESP_LOGI(" ", "%x", Multiaddr[i]);
            }
        } while (0);
        json_data = cJSON_GetObjectItem(json_root, "data");
        char *recv_data = cJSON_PrintUnformatted(json_data);

        size = asprintf(&jsonstring, "{\"src_addr\": \"" MACSTR "\", \"data\": %s}", MAC2STR(sta_mac), recv_data);
        // ret = mwifi_write(dest_addr, &data_type, jsonstring, size, true);
        ret = mwifi_root_write(Multiaddr, 1, &data_type, jsonstring, size, true);
        MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(ret));

    FREE_MEM:
        MDF_FREE(recv_data);
        MDF_FREE(jsonstring);
        cJSON_Delete(json_root);
    }

    MDF_LOGI("Uart handle task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

static void node_read_task(void *arg)
{
    mdf_err_t ret = MDF_OK;
    char *data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};

    uint8_t *buffer = NULL;
    size_t buffer_len = 0;

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
        ret = mwifi_read(src_addr, &data_type, &buffer, &buffer_len, 100 / portTICK_RATE_MS);
        // ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);

        if (ret == MDF_ERR_MWIFI_TIMEOUT || ret == ESP_ERR_MESH_TIMEOUT)
        {
            continue;
        }
        else if (ret != MDF_OK)
        {
            MDF_LOGW("<%s> mwifi_read", mdf_err_to_name(ret));
            goto FREE_MEM;
        }

        // ret = mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        // MDF_ERROR_CONTINUE(ret != MDF_OK, "<%s> mwifi_read", mdf_err_to_name(ret));
        // MDF_LOGI("Node receive, addr: " MACSTR ", size: %d, data: %s", MAC2STR(src_addr), size, data);

        /* forwoad to eth */
        if (s_ethernet_is_connected)
        {
            if (esp_eth_transmit(eth_handle, buffer, buffer_len) != ESP_OK)
            {
                ESP_LOGE(TAG, "Ethernet send packet failed");
            }
        }

        /* forwoad to uart */
        //uart_write_bytes(CONFIG_UART_PORT_NUM, buffer, buffer_len);
        //uart_write_bytes(CONFIG_UART_PORT_NUM, "\r\n", 2);
    FREE_MEM:
        MDF_FREE(buffer);
    }

    MDF_LOGW("Node read task is exit");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

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
    data_type.group = true;
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
                    // res = mwifi_write(wifi_sta_list.sta[0].mac, &data_type, msg.packet, msg.length, true);
                    // res = mwifi_write(Multiaddr, &data_type, msg.packet, msg.length, true);
                    res = mwifi_root_write(Multiaddr, 1, &data_type, msg.packet, msg.length, true);
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
#if CONFIG_EXAMPLE_ETH_PHY_IP101
    esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_RTL8201
    esp_eth_phy_t *phy = esp_eth_phy_new_rtl8201(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_LAN8720
    esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);
#elif CONFIG_EXAMPLE_ETH_PHY_DP83848
    esp_eth_phy_t *phy = esp_eth_phy_new_dp83848(&phy_config);
#endif
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
        // node_child_connected = true;

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
    // BaseType_t ret = xTaskCreate(eth2mesh_flow_control_task, "flow_ctl", 2048, NULL, (tskIDLE_PRIORITY + 2), NULL);
    //  if (ret != pdTRUE)
    //  {
    //      ESP_LOGE(TAG, "create flow control task failed");
    //      return ESP_FAIL;
    //  }
    return ESP_OK;
}

static void hb_task(void *args)
{
    int n = 0;
    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)MDF_MALLOC(BUF_SIZE);
    size_t size = MWIFI_PAYLOAD_LEN;
    // char *jsonstring = "ROOTHB";
    uint8_t dest_addr[MWIFI_ADDR_LEN] = {0};
    mwifi_data_type_t data_type = {0};
    // uint8_t sta_mac[MWIFI_ADDR_LEN] = {0};
    flow_control_msg_t msg;
    // esp_wifi_get_mac(ESP_IF_WIFI_STA, sta_mac);

    for (;;)
    {
        data_type.group = true;
        sprintf(test14G, "ROOTHB number %04d.", n);
        msg.packet = test14G;
        msg.length = sizeof(test14G);
        // uint8_t fuck =mwifi_is_started();
        // MDF_LOGI("进来 %d",fuck);
        if (mwifi_is_started() && node_child_connected)
        {
            mwifi_root_write(Multiaddr, 1, &data_type, msg.packet, msg.length, true);
            MDF_LOGI("%d", n);
            // MDF_ERROR_GOTO(ret != MDF_OK, FREE_MEM, "<%s> mwifi_root_write", mdf_err_to_name(ret));
        }

        vTaskDelay(1000 / portTICK_RATE_MS);
        n++;
    }
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

    GPIO_INIT();
    ESP_LOGI(TAG, "I am here1");
    ADF4351_Init(F);
    ESP_LOGI(TAG, "I am here2");
    int cnt = 0;
    SetFreq(F);
    SetFreq(F);
    SetFreq(F);

    ESP_LOGI(TAG, "Freq set.");

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
    const uint8_t group_id_list[2][6] = {{0x01, 0x00, 0x5e, 0xae, 0xae, 0xae},
                                       {0x01, 0x00, 0x5e, 0xae, 0xae, 0xaf}};
    //为了不组播自己
    const uint8_t group_id_list2[2][6] = {{0x01, 0x00, 0x5e, 0xae, 0xae, 0xad},
                                         {0x01, 0x00, 0x5e, 0xae, 0xae, 0xaf}};
    MDF_ERROR_ASSERT(esp_mesh_set_group_id((mesh_addr_t *)group_id_list2,
                                           sizeof(group_id_list) / sizeof(group_id_list[0])));

    for (int i = 0; i < 6; i++)
    {
        Multiaddr[i] = group_id_list[0][i];
    }

    /**
     * @brief Data transfer between wifi mesh devices
     */
    xTaskCreatePinnedToCore(node_read_task, "node_read_task", 4 * 1024,
                            NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY,
                            NULL, CONFIG_MDF_TASK_PINNED_TO_CORE);
    // xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
    //  NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

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

    xTaskCreate(hb_task, "hb_task", 4096, NULL, 10, NULL);
}
