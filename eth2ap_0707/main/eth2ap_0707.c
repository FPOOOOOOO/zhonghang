/* eth2ap (Ethernet to Wi-Fi AP packet forwarding) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
    版本说明:
    0505:可以ETH透传、UART透传
    0707:修改为UART透传、SPI透传，屏蔽ETH透传，增加了wifimsgtype，SPI sendbuf改为全局变量,加入9177频点
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

#include "nvs.h"

#include <stddef.h>
#include "freertos/semphr.h"
#include "lwip/dns.h"
#include "lwip/igmp.h"
#include "soc/rtc_periph.h"
#include "driver/spi_master.h"
#include "driver/spi_slave.h"
#include "esp_spi_flash.h"
#include "esp_intr_alloc.h"
#include "driver/timer.h"
#include "esp_timer.h"
#include <stdint.h>

// 9177
#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <sys/select.h>

#include "esp_vfs.h"
#include "esp_vfs_dev.h"
// 9177

PRIVILEGED_DATA static portMUX_TYPE xTaskQueueMutex = portMUX_INITIALIZER_UNLOCKED;

static const char *TAG = "eth2ap";
static const char *payload = "AThis is msg from Server";
static esp_eth_handle_t s_eth_handle = NULL;
static xQueueHandle flow_control_queue = NULL;
static bool s_sta_is_connected = false;
static bool s_ethernet_is_connected = false;
static bool ethernet2wifi_mac_status = false;
static bool ETH_INIT_DONE = false;
static bool WIFI_INIT_DONE = false;
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
static uint8_t wifimsgtype = 0;
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

// SPI para
#define GPIO_MOSI 13 // 12
#define GPIO_MISO 12 // 13
#define GPIO_SCLK 14 // 15
#define GPIO_CS 15   // 14
#define SENDER_HOST HSPI_HOST
#define RCV_HOST HSPI_HOST

WORD_ALIGNED_ATTR char sendbuf[129] = "hihu";
// 9177

WORD_ALIGNED_ATTR char test14G[129] = "hihu";
#define GPIO_OUTPUT_IO_0 25 // SPI?_CLK（IO32） SPI_MOSI（IO2）  SPINSS(IO16)
#define GPIO_OUTPUT_IO_1 16 // NSS
#define GPIO_OUTPUT_IO_2 2  // MOSI
#define GPIO_OUTPUT_IO_3 32 // CLK
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1) | (1ULL << GPIO_OUTPUT_IO_2) | (1ULL << GPIO_OUTPUT_IO_3))
// #define GPIO_INPUT_IO_0 34
// #define GPIO_INPUT_IO_1 5
// #define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

#define BR_CE_Set (gpio_set_level(GPIO_OUTPUT_IO_0, 1))
#define BR_CE_Clr (gpio_set_level(GPIO_OUTPUT_IO_0, 0))

#define BR_LE_Set (gpio_set_level(GPIO_OUTPUT_IO_1, 1))
#define BR_LE_Clr (gpio_set_level(GPIO_OUTPUT_IO_1, 0))

#define BR_DATA_Set (gpio_set_level(GPIO_OUTPUT_IO_2, 1))
#define BR_DATA_Clr (gpio_set_level(GPIO_OUTPUT_IO_2, 0))

#define BR_CLK_Set (gpio_set_level(GPIO_OUTPUT_IO_3, 1))
#define BR_CLK_Clr (gpio_set_level(GPIO_OUTPUT_IO_3, 0))

static xQueueHandle gpio_evt_queue = NULL;
uint32_t FREF_DIV = 500; // 输入频率(MHz)/（FREF_DIV）=0.1(MHz)
uint8_t VCO_DIV = 1;
bool VCO = 0;
uint16_t F = 225; // 120——1500M 112——1400M 104——1300M（25M晶振
                  // 250 2.5G 200 2G 300 1.5G 220 1.1G 240 0.6G （20M晶振
                  // 100 1G  150  1.5G  100 2G （40M晶振
// 9177

// SPI end

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

// 9177
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

void BR9177_Wdata(uint32_t dat)
{
    uint8_t i;
    BR_LE_Clr; //让32位移位寄存器准备接受下一次数据
    usleep(15);
    for (i = 0; i < 32; i++)
    {
        if (dat & 0x80000000)
            BR_DATA_Set;
        else
            BR_DATA_Clr;

        BR_CLK_Clr; //上升沿采样
        usleep(15);
        dat <<= 1;
        BR_CLK_Set;
        usleep(15);

        // BR_CLK_Set; //下降沿采样
        // usleep(15);
        // dat <<= 1;
        // BR_CLK_Clr;
        // usleep(15);
    }
    BR_CLK_Clr; //上升沿采样
    // BR_CLK_Set;//下降沿采样
    usleep(15);
    BR_LE_Set; //打高使得32位移位寄存器按?的寄存器地址进行写入
}

void SetFreq(uint16_t F)
{
    // uint16_t INT=500;
    // ESP_LOGI(TAG, "I am here 3 to SetFreq");
    // if(499<F && F<801){
    //     INT=F*4/10;
    // }else if(899<F && F <1601){
    //     INT=F*2/10;
    // }else if(1699<F && F<3201){
    //     INT=F/10;
    // }
    // uint32_t reg5=0xa000000a;
    // uint32_t reg4=0x86200000 | ((FREF_DIV<<7)&0x1FFF80) | VCO_DIV ;
    // uint32_t reg3=0x6075c800 | (VCO<<24)

    // //543210
    if (F == 200)
    {                             // 20M 2G
        BR9177_Wdata(0xa000000a); // 设置寄存器5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200101); // 设置寄存器4 : R预分频=1 RF分频器=2 低功率  1000 0110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // 设置寄存器3	：高频段VCO 默认             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // 设置寄存器2：MUXOUT:默认 低相噪 默认       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	设置寄存器1 ：预分频器设为4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // 设置寄存器0：3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 300)
    {                             // 20M 1.5G
        BR9177_Wdata(0xa000000a); // 设置寄存器5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // 设置寄存器4 : R预分频=1 RF分频器=2 低功率  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // 设置寄存器3	：高频段VCO 默认             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // 设置寄存器2：MUXOUT:默认 低相噪 默认       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	设置寄存器1 ：预分频器设为4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // 设置寄存器0：3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 220)
    {                             // 20M 1.1G
        BR9177_Wdata(0xa000000a); // 设置寄存器5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // 设置寄存器4 : R预分频=1 RF分频器=2 低功率  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // 设置寄存器3	：高频段VCO 默认             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // 设置寄存器2：MUXOUT:默认 低相噪 默认       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	设置寄存器1 ：预分频器设为4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // 设置寄存器0：3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 240)
    {                             // 20M 0.6G
        BR9177_Wdata(0xa000000a); // 设置寄存器5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200104); // 设置寄存器4 : R预分频=1 RF分频器=2 低功率  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // 设置寄存器3	：高频段VCO 默认             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // 设置寄存器2：MUXOUT:默认 低相噪 默认       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	设置寄存器1 ：预分频器设为4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // 设置寄存器0：3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 250)
    {                             // 20M 2.5G
        BR9177_Wdata(0xa000000a); // 设置寄存器5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200101); // 设置寄存器4 : R预分频=1 RF分频器=2 低功率  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // 设置寄存器3	：高频段VCO 默认             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // 设置寄存器2：MUXOUT:默认 低相噪 默认       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	设置寄存器1 ：预分频器设为4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // 设置寄存器0：3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 160)
    {                             // 25M 2G
        BR9177_Wdata(0xa000000a); // 设置寄存器5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200101); // 设置寄存器4 : R预分频=1 RF分频器=2 低功率  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // 设置寄存器3	：高频段VCO 默认             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // 设置寄存器2：MUXOUT:默认 低相噪 默认       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	设置寄存器1 ：预分频器设为4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // 设置寄存器0：3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    // else if (F == 100)
    // {                             // 40M 2G
    //     BR9177_Wdata(0xa000000a); // 设置寄存器5                             1010 0000 0000 0000 0000 0000 0000 1010
    //     usleep(1000);
    //     BR9177_Wdata(0x86200101); // 设置寄存器4 : R预分频=1 RF分频器=2 低功率  1000 1110 0010 0000 0000 0000 1000 0010
    //     usleep(1000);
    //     BR9177_Wdata(0x6075c800); // 设置寄存器3	：高频段VCO 默认             0110 0001 0111 0101 1100 1000 0
    //     usleep(1000);
    //     BR9177_Wdata(0x43000000); // 设置寄存器2：MUXOUT:默认 低相噪 默认       0100 0011 0000
    //     usleep(1000);
    //     BR9177_Wdata(0x29000000); //	设置寄存器1 ：预分频器设为4/5 + FRAC 0    0010 1001 0
    //     usleep(1000);
    //     BR9177_Wdata(0x05200000 | F); // 设置寄存器0：3200uA+INT                 0000 0101 1000 0
    //     usleep(1000);
    // }
    else if (F == 150)
    {                             // 40M 1.5G
        BR9177_Wdata(0xa000000a); // 设置寄存器5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // 设置寄存器4 : R预分频=1 RF分频器=2 低功率  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // 设置寄存器3	：高频段VCO 默认             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // 设置寄存器2：MUXOUT:默认 低相噪 默认       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	设置寄存器1 ：预分频器设为4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // 设置寄存器0：3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 100)
    {                             // 40M 1G
        BR9177_Wdata(0xa000000a); // 设置寄存器5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // 设置寄存器4 : R预分频=1 RF分频器=2 低功率  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // 设置寄存器3	：高频段VCO 默认             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // 设置寄存器2：MUXOUT:默认 低相噪 默认       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	设置寄存器1 ：预分频器设为4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // 设置寄存器0：3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
        else if (F == 225)
    {                             // 40M 3G
        BR9177_Wdata(0xa000000a); // ?????5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200181); // ?????4 : R???=1 RF???=2 ???  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // ?????3	????VCO ??             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ?????2?MUXOUT:?? ??? ??       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	?????1 ???????4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ?????0?3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 40)
    {                             // 50M 1G
        BR9177_Wdata(0xa000000a); // 设置寄存器5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // 设置寄存器4 : R预分频=1 RF分频器=2 低功率  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // 设置寄存器3	：高频段VCO 默认             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // 设置寄存器2：MUXOUT:默认 低相噪 默认       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	设置寄存器1 ：预分频器设为4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // 设置寄存器0：3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }

    ESP_LOGI(TAG, "Now Frequency is %d M:", F);
}
// 9177

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

static void SetApStaticip(void)
{
    int ret = esp_netif_dhcps_stop(ap_netif);
    printf("stop dhcp ret = %x\n", ret);
    char *ap_ip = "192.168.0.2";
    char *ap_gateway = "192.168.0.1";
    char *ap_netmask = "255.255.255.0";
    esp_netif_ip_info_t info_t;
    memset(&info_t, 0, sizeof(esp_netif_ip_info_t));
    ip4addr_aton((const char *)ap_ip, &info_t.ip.addr);
    ip4addr_aton((const char *)ap_gateway, &info_t.gw.addr);
    ip4addr_aton((const char *)ap_netmask, &info_t.netmask.addr);
    esp_netif_set_ip_info(ap_netif, &info_t);
}

// Forward packets from Wi-Fi to Ethernet
static esp_err_t pkt_wifi2eth(void *buffer, uint16_t len, void *eb)
{
    ESP_LOGI(TAG, "LEN =: %d", len);
    memcpy((uint16_t *)&recv_header, buffer + 12, 2);
    memcpy((uint8_t *)&wifimsgtype, buffer + 14, 1);
    recv_header = ntohs(recv_header);
    if (recv_header == 0xA55A)
    {
        ESP_LOGI(TAG, "RECV_HEADER IS: %x", recv_header);
        uint8_t *wifi_data = (uint8_t *)malloc(len - 23);
        memcpy(wifi_data, buffer + 21, len - 23);
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
        if (wifimsgtype == UART)
        {
            printf("UART:\n");
        }
        else if (wifimsgtype == SPI)
        {
            printf("SPI:\n");
            memcpy(sendbuf, wifi_data, len - 23);
        }
        uart_write_bytes(EX_UART_NUM, (const char *)wifi_data, len - 23);

        // for (int i = 0; i < len - 11; i++)
        // {
        //     printf("%c", ((char *)wifi_data)[i]);
        // }
        free(wifi_data);
        // printf("\n\r");
    }
    // TEST PART，including UDP PART
    // if (len > 60)
    // {
    //     if (len == 70)
    //     {
    //         memcpy(read_buff, buffer + 20, len - 20); // source data
    //         for (int i = 0; i < len - 20; i++)
    //         {
    //             printf("%c", ((char *)read_buff)[i]);
    //         }
    //         // ESP_LOGI(TAG, "Recv by WIFI: %s", read_buff);
    //     }
    //     else
    //     {
    //         memcpy((esp_ip4_addr_t *)&stationip[0], buffer + 26, 4); // source ip
    //         memcpy((uint16_t *)&port[0], buffer + 36, 2);            // source port
    //         memcpy(read_buff, buffer + 42, len - 42);                // source data
    //         ESP_LOGI(TAG, "Recv socket by WIFI: %s", read_buff);
    //         ESP_LOGI(TAG, "This message is from:%s", ip4addr_ntoa((ip4_addr_t *)&stationip[0]));
    //         ESP_LOGI(TAG, "Port is:%d", ntohs(port[0]));
    //     }
    // }

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
    ESP_LOGI(TAG, "ESPETH HAS GOT MSG LEN:%d", len);
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
            if (!ethernet2wifi_mac_status && WIFI_INIT_DONE)
            {
                memcpy(s_eth_mac, (uint8_t *)msg.packet + 6, sizeof(s_eth_mac)); // added from eth2wifi
                ESP_LOGI(TAG, "Ethernet s_eth_mac Addr REPLACE TIME %02x:%02x:%02x:%02x:%02x:%02x",
                         s_eth_mac[0], s_eth_mac[1], s_eth_mac[2], s_eth_mac[3], s_eth_mac[4], s_eth_mac[5]);
                ESP_ERROR_CHECK(esp_wifi_start());
                esp_wifi_set_mac(WIFI_IF_AP, s_eth_mac); // reset the sta eth once more.added
                ethernet2wifi_mac_status = true;
            }
            if (s_sta_is_connected && msg.length)
            {
                // printf("I am bad guys \n\r");
                do
                {
                    vTaskDelay(pdMS_TO_TICKS(timeout));
                    timeout += 2;
                    // memcpy(&whatinit, msg.packet, 1);
                    // printf("What's in it:%x", whatinit);
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

// static void spi_task(void *pvParameters)
// {
//     int n = 0;
//     esp_err_t ret;

//     // Configuration for the SPI bus
//     spi_bus_config_t buscfg = {
//         .mosi_io_num = GPIO_MOSI,
//         .miso_io_num = GPIO_MISO,
//         .sclk_io_num = GPIO_SCLK,
//         .flags = SPICOMMON_BUSFLAG_IOMUX_PINS, // added IOMUX
//         .quadwp_io_num = -1,                   // added -1 default
//         .quadhd_io_num = -1,
//     };

//     // Configuration for the SPI slave interface
//     spi_slave_interface_config_t slvcfg = {
//         .mode = 0,
//         .spics_io_num = GPIO_CS,
//         .queue_size = 3,
//         .flags = 0,
//         .post_setup_cb = NULL,
//         .post_trans_cb = NULL};

//     // Configuration for the handshake line
//     //  gpio_config_t io_conf={
//     //      .intr_type=GPIO_INTR_DISABLE,
//     //      .mode=GPIO_MODE_OUTPUT,
//     //      .pin_bit_mask=(1<<GPIO_HANDSHAKE)
//     //  };

//     // Configure handshake line as output
//     //  gpio_config(&io_conf);
//     // Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.
//     gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
//     gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
//     gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);

//     // Initialize SPI slave interface
//     // ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, 1);
//     ret = spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
//     ESP_LOGE(TAG, "SPI SLAVE INIT. (%s)", esp_err_to_name(ret));
//     assert(ret == ESP_OK);

//     ESP_LOGE(TAG, "I am 1");
//     // WORD_ALIGNED_ATTR char sendbuf[129] = "hihu";
//     WORD_ALIGNED_ATTR char recvbuf[129] = "fuckme";
//     memset(recvbuf, 0, 33);
//     spi_slave_transaction_t t;
//     memset(&t, 0, sizeof(t));
//     memset(recvbuf, 0x23, 129);
//     ESP_LOGE(TAG, "I am 2");

//     while (1)
//     {
//         // Clear receive buffer, set send buffer to something sane
//         // memset(recvbuf, 0xA5, 129);
//         ESP_LOGE(TAG, "I am 3");
//         int res = sprintf(sendbuf, "Htis is from receiver, number %04d.", n);
//         //?????¨
//         if (res >= sizeof(sendbuf))
//         {
//             printf("Data truncated\n");
//         }
//         ESP_LOGE(TAG, "I am 4");
//         // Set up a transaction of 128 bytes to send/receive
//         t.length = 128 * 8;
//         t.tx_buffer = sendbuf;
//         t.rx_buffer = recvbuf;
//         /* This call enables the SPI slave interface to send/receive to the sendbuf and recvbuf. The transaction is
//         initialized by the SPI master, however, so it will not actually happen until the master starts a hardware transaction
//         by pulling CS low and pulsing the clock etc. In this specific example, we use the handshake line, pulled up by the
//         .post_setup_cb callback that is called as soon as a transaction is ready, to let the master know it is free to transfer
//         data.
//         */
//         // ESP_LOGI(TAG, "isReady is: %d \n\r",isReady);

//         ret = spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY);
//         ESP_LOGE(TAG, "I am 5");
//         ESP_LOGE(TAG, "SPI. (%s)", esp_err_to_name(ret)); //   Equals  spi_slave_queue_trans() + spi_slave_get_trans_results
//         // ESP_LOGI(TAG, "isReady is: %d \n\r",isReady);

//         // esp_err_t ret;
//         // spi_slave_transaction_t *ret_trans;
//         // ToDo: check if any spi transfers in flight

//         // ret = spi_slave_queue_trans(RCV_HOST, &t, portMAX_DELAY);
//         // ESP_LOGE(TAG, "SPI_SLAVE_QUEUE_TRANS. (%s)", esp_err_to_name(ret));
//         // vTaskDelay(pdMS_TO_TICKS(100));
//         // ret = spi_slave_get_trans_result(RCV_HOST, &ret_trans, portMAX_DELAY);
//         // ESP_LOGE(TAG, "SPI_SLAVE_GET_TRANS_RESULT. (%s)", esp_err_to_name(ret));
//         // vTaskDelay(pdMS_TO_TICKS(100));
//         // assert(ret_trans == &t);
//         // return ESP_OK;

//         // spi_slave_transmit does not return until the master has done a transmission, so by here we have sent our data and
//         // received data from the master. Print it.
//         printf(" %d Received: %s\n", n, recvbuf);
//         // added 0713 to transfer via wifi
//         uint8_t SPIlength = sizeof(recvbuf);
//         uint8_t *spi2wifi_data = (uint8_t *)malloc(SPIlength + 11);
//         uint8_t *ARP_spi2wifi_data = (uint8_t *)malloc(SPIlength + 23);
//         bzero(spi2wifi_data, SPIlength + 11);
//         bzero(ARP_spi2wifi_data, SPIlength + 23);

//         memcpy(ARP_spi2wifi_data, ARP_header, 12);
//         memcpy(ARP_spi2wifi_data + 6, s_eth_mac, 6);

//         hjypackup(SPI, 0x0000000000000001, 0x00, SPIlength, recvbuf, spi2wifi_data);

//         esp_err_t ret = ESP_OK;

//         memcpy(ARP_spi2wifi_data + 12, spi2wifi_data, SPIlength + 11);
//         flow_control_msg_t msg = {
//             .packet = ARP_spi2wifi_data,
//             .length = SPIlength + 23};
//         if (xQueueSend(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) != pdTRUE)
//         {
//             // printf("eth2wifi is here:%s**********\n\r",buffer);
//             ESP_LOGE(TAG, "send flow control message failed or timeout");
//             free(spi2wifi_data);
//             ret = ESP_FAIL;
//         }

//         n++;
//         // vTaskDelay(pdMS_TO_TICKS(1000));
//         //  vTaskDelay(pdMS_TO_TICKS(2000));
//         if (n == 200)
//         {
//             printf("Already done : %d\n", n);
//             n = 0;
//         }
//     }
// }

static void hb_task(void *args)
{
    int n = 0;
    for (;;)
    {
        sprintf(test14G, "Atis is from receiver What is happening What is happening, number %04d.", n);
        flow_control_msg_t msg = {
            .packet = test14G,
            .length = sizeof(test14G)};
        if (s_sta_is_connected)
        {
            esp_wifi_internal_tx(WIFI_IF_AP, msg.packet, msg.length);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        ESP_LOGE(TAG, "ETHECHO");
        n++;
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
        ESP_LOGI(TAG, "Ethernet s_eth_mac Addr INIT TIME %02x:%02x:%02x:%02x:%02x:%02x",
                 s_eth_mac[0], s_eth_mac[1], s_eth_mac[2], s_eth_mac[3], s_eth_mac[4], s_eth_mac[5]);
        // ESP_ERROR_CHECK(esp_wifi_set_mac(WIFI_IF_AP, s_eth_mac)); // found! this is where yingshe mac address
        // ESP_ERROR_CHECK(esp_wifi_start());
        ETH_INIT_DONE = true;
        ESP_LOGI(TAG, "ETHERNET_EVENT_CONNECTED DONE\n\r");
        // esp_netif_get_ip_info(ap_netif, &ip_info);
        // ESP_LOGI(TAG, "THis AP has got an IP as:%s", ip4addr_ntoa((ip4_addr_t*)&ip_info.ip));
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        s_ethernet_is_connected = false;
        ethernet2wifi_mac_status = false;
        ETH_INIT_DONE = false;
        ESP_ERROR_CHECK(esp_wifi_stop());
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    // case IP_EVENT_ETH_GOT_IP:
    //     ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    //     const esp_netif_ip_info_t *ip_info = &event->ip_info;

    //     ESP_LOGI(TAG, "Ethernet Got IP Address");
    //     ESP_LOGI(TAG, "~~~~~~~~~~~");
    //     ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    //     ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    //     ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    //     ESP_LOGI(TAG, "~~~~~~~~~~~");
    //     break;
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
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
        //这里是不开板子的DHCPer服务
        if (!s_con_cnt)
        {
            s_sta_is_connected = true;
            // esp_wifi_set_promiscuous_rx_cb(pkt_wifi2eth);
            WIFI_INIT_DONE = true;
            esp_wifi_internal_reg_rxcb(WIFI_IF_AP, pkt_wifi2eth);
        }
        s_con_cnt++;
        ESP_LOGI(TAG, "Afer joining,now has %d STA connection", s_con_cnt);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        ESP_LOGI(TAG, "Wi-Fi AP got a station disconnected");
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
        s_con_cnt--;
        ESP_LOGI(TAG, "After leaving,now has %d STA connection", s_con_cnt);
        if (!s_con_cnt)
        {
            s_sta_is_connected = false;
            WIFI_INIT_DONE = false;
            esp_wifi_internal_reg_rxcb(WIFI_IF_AP, NULL);
        }
    }
}

static void initialize_ethernet(void)
{
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, eth_event_handler, NULL));

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO;
    mac_config.smi_mdc_gpio_num = CONFIG_EXAMPLE_ETH_MDC_GPIO;
    mac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);
    //esp_eth_phy_t *phy = esp_eth_phy_new_rtl8201(&phy_config);

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

    esp_eth_ioctl(s_eth_handle, ETH_CMD_S_PROMISCUOUS, (void *)true); //混杂模式（英语：promiscuous mode）是电脑网络中的术语。是指一台机器的网卡能够接收所有经过它的数据流，而不论其目的地址是否是它。
    /* attach Ethernet driver to TCP/IP stack */
    // ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(s_eth_handle)));
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
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
    //                                                     ESP_EVENT_ANY_ID,
    //                                                     &wifi_event_handler,
    //                                                     NULL,
    //                                                     NULL));

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
    //ESP_ERROR_CHECK(esp_wifi_start());

    // vTaskDelay(2000 / portTICK_PERIOD_MS);
    // ESP_ERROR_CHECK(esp_netif_dhcps_stop(ap_netif));
    // SetApStaticip();
    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             CONFIG_EXAMPLE_WIFI_SSID, CONFIG_EXAMPLE_WIFI_PASSWORD, CONFIG_EXAMPLE_WIFI_CHANNEL);
    WIFI_INIT_DONE = true;

    // esp_err_t rett = esp_netif_get_ip_info(ap_netif, &ip_info);
    // printf("get_ip_info ret = %u\n", rett);
    // ESP_LOGI(TAG, "This AP's ip is %s.", ip4addr_ntoa((ip4_addr_t *)&ip_info.ip));
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

    // GPIO_INIT();
    // ESP_LOGI(TAG, "I am here1");
    // // BR4351_Init(F);
    // ESP_LOGI(TAG, "I am here2");
    // BR_CE_Set;
    // usleep(1000);

    // BR_LE_Set;  //???CS????
    // BR_CLK_Clr; //?????????????
    // // BR_CLK_Set;//?????????????
    // BR_DATA_Clr;

    // int cnt = 0;
    // usleep(1000);
    // ESP_LOGI(TAG, "Start Set Freq:");
    // SetFreq(F);
    // ESP_LOGI(TAG, "Freq set.");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(initialize_flow_control());

    xTaskCreate(uart_task, "uart_task", 2048, NULL, 12, NULL);

    initialize_ethernet();
    initialize_wifi();

    esp_netif_get_ip_info(ap_netif, &ip_info);
    ESP_LOGI(TAG, "THis AP has got an IP as:%s", ip4addr_ntoa((ip4_addr_t *)&ip_info.ip));
    //xTaskCreate(hb_task, "hb_task", 4096, NULL, 10, NULL);
    //  Create a task to handler UART event from ISR
}
