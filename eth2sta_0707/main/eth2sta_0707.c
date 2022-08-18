/* eth2ap (Ethernet to Wi-Fi AP packet forwarding) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
    版本说明:
    0505:可以电脑-板子上网，Ethernet-kit稳定、Devkit+开发板不稳定。设不设置板子的静态无所谓,配合sender_UART_0505
    0620:增加了UDP封包，Qt可以收到。
    0707:修改为UART透传、SPI透传，屏蔽ETH透传,因为没有从ETH拿mac了，所以s_ethmac要写死,增加了wifimsgtype,SPI sendbuf 改为全局变量,修改9177频点
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
#include "sys/param.h"
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

static const char *TAG = "eth2sta";
static const char *payload = "This is msg from STA Client";
static const char *statusinfo = "Message from ESP32 ";
static esp_eth_handle_t s_eth_handle = NULL;
static xQueueHandle flow_control_queue = NULL;
static bool s_sta_is_connected = false;
static bool s_ethernet_is_connected = false;
static bool WIFI_INIT_DONE = false;
static bool ethernet2wifi_mac_status = false;
// static uint8_t s_eth_mac[6] = {0x24, 0xd7, 0xeb, 0xbb, 0x9a, 0x6c}; // ESPgood as STA 6c as AP 6d
// static uint8_t s_eth_mac[6]={0x8c,0x4b,0x14,0x14,0x77,0xb8};//ESPbad as STA b8 as AP b9
static uint8_t s_eth_mac[6] = {0x78, 0x21, 0x84, 0x70, 0x93, 0x78}; // OURS as STA 78 as AP 79
//  static uint8_t s_mac[6]={0x00,0x0e,0xc6,0x78,0xf1,0x02};
static uint8_t s_mac[6] = {0x8c, 0x4b, 0x14, 0x14, 0x77, 0xbb};

// static uint8_t d_mac[6]={0xdc,0xa9,0x04,0x89,0x5f,0x86};//this is mac wifi
// static uint8_t d_mac[6]={0x00,0x0e,0xc6,0x78,0xf1,0x02};
static uint8_t d_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // this is mac wifi
static esp_netif_t *ap_netif = NULL;
esp_netif_t *eth_netif;
esp_netif_ip_info_t ip_info;
struct sockaddr_in local_addr, dest_addr;
esp_ip4_addr_t stationip[16];
uint16_t port[16];
char read_buff[1500]; //接收数据的缓存数组
static uint8_t fakee[70] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0xd8, 0x61, 0x71, 0xc3, 0x6f,
    0x08, 0x06, 0x00, 0x01, 0x08, 0x00,
    0x06, 0x04, 0x00, 0x01,
    0x00, 0xd8, 0x61, 0x71, 0xc3, 0x6f,
    0xc0, 0xa8, 0x04, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x04, 0x01}; //伪造ARP包
static uint8_t fake[70] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x0e, 0xc6, 0x78, 0xf1, 0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x08, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x01}; //伪造ARP包
static uint8_t ARP_header[12] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// package
static const uint16_t header = 0xA55A;
#define UART (uint8_t)4
#define SPI (uint8_t)7
static uint8_t wifimsgtype = 0;
#define ETH_LENGTH 6
//#define STA1 0x00000001
static uint16_t recv_header = 0x0000;

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
#define GPIO_CS 15   // 14 SPI for ESP

// #define GPIO_MOSI 13 // 12
// #define GPIO_MISO 12 // 13
// #define GPIO_SCLK 14 // 15
// #define GPIO_CS 15   // 14 SPI for OURS

WORD_ALIGNED_ATTR char sendbuf[129] = "hihu";

#define SENDER_HOST HSPI_HOST
#define RCV_HOST HSPI_HOST

// SPI end

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
                  // 250 2.5G 200 2G 300 1.5G 220 1.1G 240 0.6G  300 3G 350 3.5G（20M晶振
                  // 100 1G  150  1.5G  100 2G 225 3G（40M晶振
// 9177

int sock;
// UDP全局变量

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
// added from socket
#define PORT CONFIG_EXAMPLE_PORT1
#define PORT_SPI CONFIG_EXAMPLE_PORT2
#define PORT_UART CONFIG_EXAMPLE_PORT3

#if defined(CONFIG_EXAMPLE_IPV4)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#elif defined(CONFIG_EXAMPLE_IPV6)
#define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#else
#define HOST_IP_ADDR ""
#endif 

// static ip start

#define EXAMPLE_STATIC_IP_ADDR "192.168.4.7"
#define EXAMPLE_STATIC_NETMASK_ADDR "255.255.255.0"
#define EXAMPLE_STATIC_GW_ADDR "192.168.4.1"

#define EXAMPLE_MAIN_DNS_SERVER EXAMPLE_STATIC_GW_ADDR
#define EXAMPLE_BACKUP_DNS_SERVER "0.0.0.0"

// static ip end

// static const char *payload = "Message from ETHBoard";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group; // only used in station mode

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
        BR9177_Wdata(0xa000000a); // 设置寄存器5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200181); // 设置寄存器4 : R预分频=1 RF分频器=2 低功率  1000 1110 0010 0000 0000 0000 1000 0010
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



// #region UDP封包部分

#pragma pack(1) //单字节对其
//数据链路层
// des 6byte + src 6byte + type 2byte + data(46~1500byte) + crc 4byte
typedef struct
{
    unsigned char des_mac[ETH_LENGTH];
    unsigned char src_mac[ETH_LENGTH];
    unsigned short type; //协议类型，ip协议0x0800
} link_layer_header_def;

//网络层
// 20byte
typedef struct
{
    unsigned char versionANDheader_len;
    // unsigned char header_len : 4;

    unsigned char tos;
    unsigned short total_len;

    unsigned short id;
    unsigned short flag : 3;
    unsigned short offset : 13;

    unsigned char ttl;
    unsigned char protocol_num;
    unsigned short checksum;

    unsigned int src_ip;
    unsigned int des_ip;
} network_layer_header_def;

typedef struct
{
    unsigned short hw_type; //
    unsigned short proto_type;

    unsigned char hw_addr_len;
    unsigned char proto_len;
    unsigned short op;

    unsigned char s_mac[ETH_LENGTH];
    unsigned int s_ip;
    unsigned char d_mac[ETH_LENGTH];
    unsigned int d_ip;
} network_layer_arp_header_def;

//传输层
// udp src port 2byte + udp len 2byte +des port 2byte + crc 2byte = 8byte
typedef struct
{
    unsigned short src_port;
    unsigned short des_port;
    unsigned short len;
    unsigned short crc;
} udp_transport_layer_header_def;

typedef struct
{
    unsigned char type;
    unsigned char code;
    unsigned short sum;

    unsigned short id;
    unsigned short num;
} icmp_transport_layer_header_def;
#pragma pack()

#pragma pack(1)
//用户数据
typedef struct
{
    //头部定义
    link_layer_header_def eh;           // 14 byte
    network_layer_header_def ip;        // 20 byte
    udp_transport_layer_header_def udp; // 8 byte

    //数据定义
    unsigned char body[0];
} udp_packet_def;

typedef struct
{
    //头部定义
    link_layer_header_def eh;             // 14 byte
    network_layer_header_def ip;          // 20 byte
    icmp_transport_layer_header_def icmp; // 8 byte

    //数据定义
    unsigned char body[0];
} ping_packet_def;
#pragma pack()

//主机字节转网络字节
unsigned short h_net(unsigned short x)
{
    unsigned short tmp;

    tmp = x & 0xff;
    tmp <<= 8;
    tmp += (x >> 8);

    return tmp;
}

//网络字节转主机字节
unsigned short n_host(unsigned short x)
{
    unsigned short tmp;

    tmp = x & 0xff;
    tmp <<= 8;
    tmp += (x >> 8);

    return tmp;
}

unsigned short check_sum(unsigned char *s, int len)
{
    unsigned int tmp = 0, i;
    unsigned short *p_tmp16, *p1_tmp16, ret;

    p_tmp16 = s;
    p1_tmp16 = &tmp;

    for (i = 0; i < (len / 2); i++)
    {
        tmp += p_tmp16[i];
    }

    if ((len % 2) == 1)
    {
        tmp += s[(len - 1)];
    }

    if (p1_tmp16[1] > 0)
    {
        tmp = (tmp & 0xffff) + p1_tmp16[1];
    }
    ret = ((~tmp) & 0xffff);

    return ret;
}

unsigned short ip_check_sum(unsigned short *s, int len)
{ // calculate with Internet Seq
    unsigned long checksum = 0;
    unsigned short temp = 0;
    while (len > 1)
    {
        temp = ntohs(*(unsigned short *)s);
        // printf("%x ",temp);
        checksum += temp;
        s++;
        len -= 2;
    }
    if (len)
    {
        checksum += *(unsigned char *)s;
    }
    checksum = (checksum >> 16) + (checksum & 0xffff);
    // checksum+=(checksum>>16);
    return (unsigned short)(~checksum);
}

//十进制ip转字符串
// int ip_dec_str(unsigned char *ip, unsigned char *str)
// {
// 	sprintf(str, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
// 	printf("%s\n",str);
// 	return 0;
// }

//字符串ip转十进制
int ip_str_dec(unsigned char *str, unsigned char *ip)
{
    int i, j;
    unsigned char tmp = 0, *ip_tmp, len;

    ip_tmp = ip;

    for (i = 0; i < 4; i++)
    {
        tmp = 0;
        for (j = 0; j < 4; j++)
        {
            if (*str == '.' || *str == '\0')
            {
                str++;
                break;
            }
            tmp *= 10;
            tmp += ((*str) - '0');
            str++;
            len--;
        }
        *ip_tmp = tmp;
        printf("tmp: %d\n", *ip_tmp);
        ip_tmp++;
    }
    printf("ip %d %d %d %d\n", ip[0], ip[1], ip[2], ip[3]);

    return 0;
}

// udp封包函数
int udp_packet(unsigned char *pkt, unsigned char *sip, unsigned short sport, unsigned char *dip, unsigned short dport, unsigned char *data, unsigned int data_len)
{
    udp_packet_def *udp;
    unsigned char *p_tmp;
    unsigned short crc_tmp;
    int i;

    // printf("获取地址：\n");
    udp = (udp_packet_def *)pkt;
    // printf("组en头：\n");
    memcpy(&(udp->eh.src_mac), s_mac, 6);
    memcpy(&(udp->eh.des_mac), d_mac, 6);
    udp->eh.type = ntohs(0x0800); // 0x0600 XNS 0x0800 IP 0x0806 ARP 0x6003 DECnet

    // printf("组ip头：\n");
    udp->ip.versionANDheader_len = 4 << 4 | 5; // 4 ipv4 6为ipv6 ipv4 默认5x4 20个 字节
    udp->ip.tos = 0;
    // udp->ip.total_len = h_net(data_len + 8 + 20);
    udp->ip.total_len = h_net(data_len + 8 + 20);

    udp->ip.id = ntohs(0x0003); //
    udp->ip.flag = 0x00;
    udp->ip.offset = 0x0000;

    udp->ip.ttl = 255;
    udp->ip.protocol_num = 17; // udp 17 tcp 6
    udp->ip.checksum = 0;

    p_tmp = &(udp->ip.src_ip);
    memcpy(p_tmp, sip, 4);

    p_tmp = &(udp->ip.des_ip);
    memcpy(p_tmp, dip, 4);

    crc_tmp = ip_check_sum(&udp->ip, 20);
    // udp->ip.checksum =ntohs(0xc0ca);
    udp->ip.checksum = ntohs(crc_tmp);

    // printf("组udp头：\n");
    udp->udp.des_port = ntohs(dport);
    udp->udp.src_port = ntohs(sport);
    udp->udp.len = h_net(data_len + 8);
    udp->udp.crc = 0;

    // printf("填入数据：\n");
    memcpy(udp->body, data, data_len);

    // printf("计算校验数据：\n");
    // crc_tmp =  check_sum(&udp->ip.src_ip, (data_len + 16));
    // printf("计算crc：%x\n", crc_tmp);

    return 0;
}

// #endregion

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

//暂时是不需要参数，后续再加从机数量
static void Send_UDP(void *payload)
{
    int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
    }
    ESP_LOGI(TAG, "Message sent");
}

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

// offical static ip
static esp_err_t example_set_dns_server(esp_netif_t *netif, uint32_t addr, esp_netif_dns_type_t type)
{
    if (addr && (addr != IPADDR_NONE))
    {
        esp_netif_dns_info_t dns;
        dns.ip.u_addr.ip4.addr = addr;
        dns.ip.type = IPADDR_TYPE_V4;
        ESP_ERROR_CHECK(esp_netif_set_dns_info(netif, type, &dns));
    }
    return ESP_OK;
}

static void example_set_static_ip(esp_netif_t *netif)
{
    if (esp_netif_dhcpc_stop(netif) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to stop dhcp client");
        return;
    }
    esp_netif_ip_info_t ip;
    memset(&ip, 0, sizeof(esp_netif_ip_info_t));
    ip.ip.addr = ipaddr_addr(EXAMPLE_STATIC_IP_ADDR);
    ip.netmask.addr = ipaddr_addr(EXAMPLE_STATIC_NETMASK_ADDR);
    ip.gw.addr = ipaddr_addr(EXAMPLE_STATIC_GW_ADDR);
    if (esp_netif_set_ip_info(netif, &ip) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set ip info");
        return;
    }
    ESP_LOGD(TAG, "Success to set static ip: %s, netmask: %s, gw: %s", EXAMPLE_STATIC_IP_ADDR, EXAMPLE_STATIC_NETMASK_ADDR, EXAMPLE_STATIC_GW_ADDR);
    // ESP_ERROR_CHECK(example_set_dns_server(netif, ipaddr_addr(EXAMPLE_MAIN_DNS_SERVER), ESP_NETIF_DNS_MAIN));
    // ESP_ERROR_CHECK(example_set_dns_server(netif, ipaddr_addr(EXAMPLE_BACKUP_DNS_SERVER), ESP_NETIF_DNS_BACKUP));
}

// NVSSaveFunc
static void param_save(void)
{
    // Open
    printf("\n");
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        printf("Done\n");

        // Read
        printf("Reading restart counter from NVS ... ");
        int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
        err = nvs_get_i32(my_handle, "restart_counter", &restart_counter);
        switch (err)
        {
        case ESP_OK:
            printf("Done\n");
            printf("Restart counter = %d\n", restart_counter);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default:
            printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        // Write
        printf("Updating restart counter in NVS ... ");
        restart_counter++;
        err = nvs_set_i32(my_handle, "restart_counter", restart_counter);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }
}

static void udp_header(uint16_t len, uint8_t *buffer, uint16_t srcport, uint16_t dstport)
{
    buffer[0] = srcport >> 8;
    buffer[1] = srcport;
    buffer[2] = dstport >> 8;
    buffer[3] = dstport;
    buffer[4] = len >> 8;
    buffer[5] = len;
    buffer[6] = 0;
    buffer[7] = 0;
}

// Forward packets from Wi-Fi to Ethernet
static esp_err_t pkt_wifi2pkg(void *buffer, uint16_t len, void *eb)
{
    // ESP_LOGI(TAG, "LEN =: %d", len);
    memcpy((uint16_t *)&recv_header, buffer, 2);
    memcpy((uint8_t *)&wifimsgtype, buffer + 2, 1);
    recv_header = ntohs(recv_header);
    if (recv_header == 0xA55A)
    {
        ESP_LOGI(TAG, "RECV_HEADER IS: %x", recv_header);
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
        if (wifimsgtype == UART)
        {
            printf("UART:\n");
        }
        else if (wifimsgtype == SPI)
        {
            printf("SPI:\n");
        }
        uart_write_bytes(EX_UART_NUM, (const char *)wifi_data, len - 11);

        // for (int i = 0; i < len - 11; i++)
        // {
        //     printf("%c", ((char *)wifi_data)[i]);
        // }
        free(wifi_data);
        // printf("\n\r");
    }
    // if(recv_header == 0xAA55){
    //     package_msg_t recv_data;
    //     uint8_t *wifi_data = (uint8_t *)malloc(len-11);
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
    //     free(wifi_data);
    // }
    if (s_ethernet_is_connected)
    {
        // printf("pkt_wifi2eth is here\n\r ");
        if (esp_eth_transmit(s_eth_handle, buffer, len) != ESP_OK)
        {
            ESP_LOGE(TAG, "Ethernet send packet failed");
        }
    }
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
    // printf("ETH input msg long = %d \n\r",len);
    if (xQueueSend(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) != pdTRUE)
    {
        printf("eth2wifi is here:%s**********\n\r", buffer);
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
            if (!ethernet2wifi_mac_status & WIFI_INIT_DONE)
            {
                memcpy(s_eth_mac, (uint8_t *)msg.packet + 6, sizeof(s_eth_mac)); // added from eth2wifi
                ESP_LOGI(TAG, "Ethernet s_eth_mac Addr REPLACE TIME %02x:%02x:%02x:%02x:%02x:%02x",
                         s_eth_mac[0], s_eth_mac[1], s_eth_mac[2], s_eth_mac[3], s_eth_mac[4], s_eth_mac[5]);
                ESP_ERROR_CHECK(esp_wifi_start());
                esp_wifi_set_mac(WIFI_IF_STA, s_eth_mac);
                // esp_wifi_connect();                      // reset the sta eth once more.added
                ethernet2wifi_mac_status = true;
            }
            if (s_sta_is_connected && msg.length)
            {
                do
                {
                    vTaskDelay(pdMS_TO_TICKS(timeout));
                    timeout += 2;
                    // uart_write_bytes(EX_UART_NUM, (const char *)msg.packet, msg.length);
                    res = esp_wifi_internal_tx(WIFI_IF_STA, msg.packet, msg.length);
                    // ESP_LOGE(TAG, "Send wifi data. (%s)", esp_err_to_name(res));

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

static void hb_task(void *args)
{
    // ETH ECHO
    uint8_t *status_data = (uint8_t *)malloc(99);
    unsigned char sip[50] = {0};
    unsigned short sport;
    unsigned char dip[50] = {0};
    unsigned short dport;
    unsigned char data[1024] = {0};
    unsigned int data_len;
    unsigned char ip_tmp[30] = {0};
    unsigned char pkt1[52 + 10];

    ip_str_dec((unsigned char *)("192.168.4.7"), sip);
    ip_str_dec((unsigned char *)("192.168.4.88"), dip);

    for (;;)
    {
        bzero(status_data, 68);
        // udp_header(20, status_data, 1005, 318);
        udp_packet(status_data, sip, 8080, dip, 7777, statusinfo, 19);
        // ESP_LOGI(TAG, "status info len [%d]",sizeof(statusinfo));
        if (s_ethernet_is_connected)
        {
            // printf("pkt_wifi2eth is here\n\r ");
            if (esp_eth_transmit(s_eth_handle, status_data, 68) != ESP_OK)
            {
                ESP_LOGE(TAG, "Ethernet ECHO FAILED");
            }
        }
        vTaskDelay(4000 / portTICK_PERIOD_MS);
        ESP_LOGE(TAG, "ETHECHO");
    }
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
                /*case UART_DATA:
                    ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                    uart_read_bytes(EX_UART_NUM, uart_recv_data, event.size, portMAX_DELAY);
                    ESP_LOGI(TAG, "[DATA EVT]:");
                    uint8_t *uart2wifi_data = (uint8_t *)malloc(event.size + 11);
                    uint8_t *ARP_uart2wifi_data = (uint8_t *)malloc(UART_BUFSIZE + 23);
                    bzero(ARP_uart2wifi_data, UART_BUFSIZE + 23);
                    // if (ethernet2wifi_mac_status)
                    // {
                    //     memcpy(ARP_uart2wifi_data, ARP_header, 12);
                    //     memcpy(ARP_uart2wifi_data + 6, s_eth_mac, 6);
                    // }
                    memcpy(ARP_uart2wifi_data, ARP_header, 12);
                    memcpy(ARP_uart2wifi_data + 6, s_eth_mac, 6);
                    bzero(uart2wifi_data, event.size + 11);
                    hjypackup(UART, 0x0000000000000001, 0x00, event.size, uart_recv_data, uart2wifi_data);
                    // Send_UDP(uart2wifi_data);

                    memcpy(ARP_uart2wifi_data + 12, uart2wifi_data, event.size + 11);
                    // memcpy(&(fake[12]), uart2wifi_data, 58);
                    //  int ret = esp_wifi_internal_tx(WIFI_IF_STA, uart2wifi_data, event.size + 11);
                    // int ret = esp_wifi_internal_tx(WIFI_IF_STA, &(fake[0]), 70);
                    esp_err_t ret = ESP_OK;
                    flow_control_msg_t msg = {
                        .packet = ARP_uart2wifi_data,
                        .length = event.size + 23};
                    if (xQueueSend(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) != pdTRUE)
                    {
                        // printf("eth2wifi is here:%s**********\n\r",buffer);
                        ESP_LOGE(TAG, "send flow control message failed or timeout");
                        free(uart2wifi_data);
                        ret = ESP_FAIL;
                    }
                    // int ret = esp_wifi_internal_tx(WIFI_IF_STA, ARP_uart2wifi_data, event.size + 23);
                    // ESP_LOGE(TAG, "Send wifi data. (%s)", esp_err_to_name(ret));
                    // free(uart2wifi_data);
                    // ————————————↑单独发送
                    // esp_err_t ret = ESP_OK;
                    // flow_control_msg_t msg = {
                    //     .packet = uart2wifi_data,
                    //     .length = event.size + 11};
                    // if (xQueueSend(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) != pdTRUE)
                    // {
                    //     // printf("eth2wifi is here:%s**********\n\r",buffer);
                    //     ESP_LOGE(TAG, "send flow control message failed or timeout");
                    //     free(uart2wifi_data);
                    //     ret = ESP_FAIL;
                    // }

                    // int ret = esp_wifi_80211_tx(WIFI_IF_AP,dtmp,event.size,true);
                    // int ret = esp_wifi_internal_tx(WIFI_IF_AP, (void *)uart_recv_data, event.size);
                    // int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                    //  if (err < 0) {
                    //      ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    //      break;
                    //  }
                    // ESP_LOGE(TAG, "Send wifi data. (%s)", esp_err_to_name(ret));
                    uart_write_bytes(EX_UART_NUM, (const char *)uart_recv_data, event.size);

                    break;*/
            case UART_DATA:
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                const int rxBytes = uart_read_bytes(EX_UART_NUM, uart_recv_data, event.size, portMAX_DELAY);
                ESP_LOGI(TAG, "[DATA EVT]:");
                uart_write_bytes(EX_UART_NUM, (const char *)uart_recv_data, event.size);
                // memcpy(&uart_data, uart_recv_data, 2);
                // FF=*uart_recv_data;
                ESP_LOGI(TAG, "uart_data is %s:", uart_recv_data);
                uint16_t uart_data = atoi((const char *)uart_recv_data);
                SetFreq(uart_data);
                // SetFreq(*uart_recv_data);
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

    ESP_LOGE(TAG, "I am 1");
    // WORD_ALIGNED_ATTR char sendbuf[129] = "hihu";
    WORD_ALIGNED_ATTR char recvbuf[129] = "fuckme";
    memset(recvbuf, 0, 33);
    spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t));
    memset(recvbuf, 0x23, 129);
    ESP_LOGE(TAG, "I am 2");

    while (1)
    {
        // Clear receive buffer, set send buffer to something sane
        // memset(recvbuf, 0xA5, 129);
        ESP_LOGE(TAG, "I am 3");
        int res = sprintf(sendbuf, "Htis is from receiver, number %04d.", n);
        //?????¨
        if (res >= sizeof(sendbuf))
        {
            printf("Data truncated\n");
        }
        ESP_LOGE(TAG, "I am 4");
        // Set up a transaction of 128 bytes to send/receive
        t.length = 128 * 8;
        t.tx_buffer = sendbuf;
        t.rx_buffer = recvbuf;
        /* This call enables the SPI slave interface to send/receive to the sendbuf and recvbuf. The transaction is
        initialized by the SPI master, however, so it will not actually happen until the master starts a hardware transaction
        by pulling CS low and pulsing the clock etc. In this specific example, we use the handshake line, pulled up by the
        .post_setup_cb callback that is called as soon as a transaction is ready, to let the master know it is free to transfer
        data.
        */
        // ESP_LOGI(TAG, "isReady is: %d \n\r",isReady);

        ret = spi_slave_transmit(RCV_HOST, &t, portMAX_DELAY);
        ESP_LOGE(TAG, "I am 5");
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
        uint8_t *spi2wifi_data = (uint8_t *)malloc(SPIlength + 11);
        uint8_t *ARP_spi2wifi_data = (uint8_t *)malloc(SPIlength + 23);
        bzero(spi2wifi_data, SPIlength + 11);
        bzero(ARP_spi2wifi_data, SPIlength + 23);

        memcpy(ARP_spi2wifi_data, ARP_header, 12);
        memcpy(ARP_spi2wifi_data + 6, s_eth_mac, 6);

        hjypackup(SPI, 0x0000000000000001, 0x00, SPIlength, recvbuf, spi2wifi_data);

        esp_err_t ret = ESP_OK;

        memcpy(ARP_spi2wifi_data + 12, spi2wifi_data, SPIlength + 11);
        flow_control_msg_t msg = {
            .packet = ARP_spi2wifi_data,
            .length = SPIlength + 23};
        if (xQueueSend(flow_control_queue, &msg, pdMS_TO_TICKS(FLOW_CONTROL_QUEUE_TIMEOUT_MS)) != pdTRUE)
        {
            // printf("eth2wifi is here:%s**********\n\r",buffer);
            ESP_LOGE(TAG, "send flow control message failed or timeout");
            free(spi2wifi_data);
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
        // example_set_static_ip(eth_netif);
        esp_eth_ioctl(s_eth_handle, ETH_CMD_G_MAC_ADDR, s_eth_mac);
        ESP_LOGI(TAG, "Ethernet s_eth_mac Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 s_eth_mac[0], s_eth_mac[1], s_eth_mac[2], s_eth_mac[3], s_eth_mac[4], s_eth_mac[5]);
        // esp_wifi_set_mac(WIFI_IF_STA, s_eth_mac); // found! this is where yingshe mac address
        //  ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(TAG, "ETHERNET_EVENT_CONNECTED DONE\n\r");
        // printf("printf here\n\r");
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
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START\r\n");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED\r\n");
        wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
        ESP_LOGI(TAG, "AP " MACSTR " HAS CONNECTED", MAC2STR(event->bssid));
        // example_set_static_ip(arg);
        s_sta_is_connected = true;
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_internal_reg_rxcb(WIFI_IF_STA, pkt_wifi2pkg);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            WIFI_INIT_DONE = false;
            param_save();
            fflush(stdout);
            esp_restart();
        }
        ESP_LOGI(TAG, "connect to the AP fail");
        s_sta_is_connected = false;
        esp_wifi_internal_reg_rxcb(WIFI_IF_STA, NULL);
    }
    // else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    // {
    //     ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    //     ESP_LOGI(TAG, "STA GOT static ip:" IPSTR, IP2STR(&event->ip_info.ip));
    //     s_retry_num = 0;
    //     s_sta_is_connected = true;
    //     xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    //     // xTaskCreate(udp_client_task, "udp_client_task", 4096, NULL, 5, NULL);
    //     esp_wifi_internal_reg_rxcb(WIFI_IF_STA, pkt_wifi2pkg);
    // }
}

static void initialize_ethernet(void)
{
    // esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    // eth_netif= esp_netif_new(&cfg);
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler, NULL));
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = CONFIG_EXAMPLE_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = CONFIG_EXAMPLE_ETH_PHY_RST_GPIO;

    mac_config.smi_mdc_gpio_num = CONFIG_EXAMPLE_ETH_MDC_GPIO;
    mac_config.smi_mdio_gpio_num = CONFIG_EXAMPLE_ETH_MDIO_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
    // esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_rtl8201(&phy_config);

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    config.stack_input = pkt_eth2wifi;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &s_eth_handle));

    esp_eth_ioctl(s_eth_handle, ETH_CMD_S_PROMISCUOUS, (void *)true); //混杂模式（英语：promiscuous mode）是电脑网络中的术语。是指一台机器的网卡能够接收所有经过它的数据流，而不论其目的地址是否是它。
    // ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(s_eth_handle)));
    ESP_ERROR_CHECK(esp_eth_start(s_eth_handle));
}

static void initialize_wifi(void)
{
    s_wifi_event_group = xEventGroupCreate();
    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    // ap_netif = esp_netif_create_default_wifi_sta();
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        sta_netif,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        sta_netif,
                                                        &instance_got_ip));

    // ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_EXAMPLE_WIFI_SSID,
            .password = CONFIG_EXAMPLE_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));               // default wifi_mode_ap
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // default wifi_if_sta

    // esp_netif_dhcpc_stop(sta_netif);
    // char *ap_ip = "192.168.4.7";
    // char *ap_gateway = "192.168.4.1";
    // char *ap_netmask = "255.255.255.0";
    // esp_netif_ip_info_t info_t;
    // memset(&info_t, 0, sizeof(esp_netif_ip_info_t));
    // ip4addr_aton((const char *)ap_ip, &info_t.ip.addr);
    // ip4addr_aton((const char *)ap_gateway, &info_t.gw.addr);
    // ip4addr_aton((const char *)ap_netmask, &info_t.netmask.addr);
    // esp_netif_set_ip_info(sta_netif, &info_t);
    // ESP_LOGI(TAG, "This sta's ip is %s.", ip4addr_ntoa((ip4_addr_t *)&info_t.ip));

    // ESP_ERROR_CHECK(esp_netif_dhcps_stop(ap_netif));

    // vTaskDelay(2000 / portTICK_PERIOD_MS);
    // esp_wifi_set_mac(WIFI_IF_STA, s_eth_mac); // found! this is where yingshe mac address
    // ESP_ERROR_CHECK(esp_wifi_start()); // moved from eth handle here
    WIFI_INIT_DONE = true;
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
                 CONFIG_EXAMPLE_WIFI_SSID, CONFIG_EXAMPLE_WIFI_PASSWORD);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 CONFIG_EXAMPLE_WIFI_SSID, CONFIG_EXAMPLE_WIFI_PASSWORD);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
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

    GPIO_INIT();
    ESP_LOGI(TAG, "I am here1");
    // BR4351_Init(F);
    ESP_LOGI(TAG, "I am here2");
    BR_CE_Set;
    usleep(1000);

    BR_LE_Set;  //???CS????
    BR_CLK_Clr; //?????????????
    // BR_CLK_Set;//?????????????
    BR_DATA_Clr;

    int cnt = 0;
    usleep(1000);
    ESP_LOGI(TAG, "Start Set Freq:");
    SetFreq(F);
    ESP_LOGI(TAG, "Freq set.");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(initialize_flow_control());

    // vTaskDelay(2000 / portTICK_PERIOD_MS);
    // ESP_LOGE(TAG, "Waiting over");

    // xTaskCreate(hb_task, "hb_task", 2048, NULL, 10, NULL);
    // xTaskCreate(spi_task, "spi_task", 4096, NULL, 12, NULL);
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 12, NULL);

    initialize_ethernet();
    initialize_wifi();

    // xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
}