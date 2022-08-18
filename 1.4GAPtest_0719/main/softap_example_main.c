/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
   0719:±‰∆µµΩ1.4G
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

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

#define GPIO_OUTPUT_IO_0 25 // SPI?_CLK£®IO32£© SPI_MOSI£®IO2£©  SPINSS(IO16)
#define GPIO_OUTPUT_IO_1 16 // NSS
#define GPIO_OUTPUT_IO_2 2  // MOSI
#define GPIO_OUTPUT_IO_3 32 // CLK
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1) | (1ULL << GPIO_OUTPUT_IO_2) | (1ULL << GPIO_OUTPUT_IO_3))
#define GPIO_INPUT_IO_0 34
#define GPIO_INPUT_IO_1 5
#define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

#define BR_CE_Set (gpio_set_level(GPIO_OUTPUT_IO_0, 1))
#define BR_CE_Clr (gpio_set_level(GPIO_OUTPUT_IO_0, 0))

#define BR_LE_Set (gpio_set_level(GPIO_OUTPUT_IO_1, 1))
#define BR_LE_Clr (gpio_set_level(GPIO_OUTPUT_IO_1, 0))

#define BR_DATA_Set (gpio_set_level(GPIO_OUTPUT_IO_2, 1))
#define BR_DATA_Clr (gpio_set_level(GPIO_OUTPUT_IO_2, 0))

#define BR_CLK_Set (gpio_set_level(GPIO_OUTPUT_IO_3, 1))
#define BR_CLK_Clr (gpio_set_level(GPIO_OUTPUT_IO_3, 0))

#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM (3) /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE (1024)
#define UART_BUFSIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
static QueueHandle_t uart0_queue;
static QueueHandle_t uart1_queue;

// The semaphore indicating the slave is ready to receive stuff.
static uint16_t uart_data = 0x0000;
static xQueueHandle rdySem;
static xQueueHandle gpio_evt_queue = NULL;
uint32_t FREF_DIV = 500; //  ‰»Î∆µ¬ (MHz)/£®FREF_DIV£©=0.1(MHz)
uint8_t VCO_DIV = 1;
bool VCO = 0;
uint16_t F = 100; // 120°™°™1500M 112°™°™1400M 104°™°™1300M£®25Mæß’Ò
                  // 250 2.5G 200 2G 300 1.5G 220 1.1G 240 0.6G £®20Mæß’Ò
                  // 100 1G  150  1.5G  100 2G £®40Mæß’Ò

static const char *TAG = "wifi softAP";
static esp_netif_t *ap_netif = NULL;
esp_netif_ip_info_t ip_info;

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
    BR_LE_Clr; //»√32Œª“∆Œªºƒ¥Ê∆˜◊º±∏Ω” ‹œ¬“ª¥Œ ˝æ›
    usleep(15);
    for (i = 0; i < 32; i++)
    {
        if (dat & 0x80000000)
            BR_DATA_Set;
        else
            BR_DATA_Clr;

        BR_CLK_Clr; //…œ…˝—ÿ≤…—˘
        usleep(15);
        dat <<= 1;
        BR_CLK_Set;
        usleep(15);

        // BR_CLK_Set; //œ¬Ωµ—ÿ≤…—˘
        // usleep(15);
        // dat <<= 1;
        // BR_CLK_Clr;
        // usleep(15);
    }
    BR_CLK_Clr; //…œ…˝—ÿ≤…—˘
    // BR_CLK_Set;//œ¬Ωµ—ÿ≤…—˘
    usleep(15);
    BR_LE_Set; //¥Ú∏ﬂ πµ√32Œª“∆Œªºƒ¥Ê∆˜∞¥?µƒºƒ¥Ê∆˜µÿ÷∑Ω¯–––¥»Î
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
        BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200101); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 0110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // …Ë÷√ºƒ¥Ê∆˜3	£∫∏ﬂ∆µ∂ŒVCO ƒ¨»œ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // …Ë÷√ºƒ¥Ê∆˜2£∫MUXOUT:ƒ¨»œ µÕœ‡‘Î ƒ¨»œ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	…Ë÷√ºƒ¥Ê∆˜1 £∫‘§∑÷∆µ∆˜…ËŒ™4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // …Ë÷√ºƒ¥Ê∆˜0£∫3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 300)
    {                             // 20M 1.5G
        BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // …Ë÷√ºƒ¥Ê∆˜3	£∫∏ﬂ∆µ∂ŒVCO ƒ¨»œ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // …Ë÷√ºƒ¥Ê∆˜2£∫MUXOUT:ƒ¨»œ µÕœ‡‘Î ƒ¨»œ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	…Ë÷√ºƒ¥Ê∆˜1 £∫‘§∑÷∆µ∆˜…ËŒ™4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // …Ë÷√ºƒ¥Ê∆˜0£∫3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 220)
    {                             // 20M 1.1G
        BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // …Ë÷√ºƒ¥Ê∆˜3	£∫∏ﬂ∆µ∂ŒVCO ƒ¨»œ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // …Ë÷√ºƒ¥Ê∆˜2£∫MUXOUT:ƒ¨»œ µÕœ‡‘Î ƒ¨»œ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	…Ë÷√ºƒ¥Ê∆˜1 £∫‘§∑÷∆µ∆˜…ËŒ™4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // …Ë÷√ºƒ¥Ê∆˜0£∫3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 240)
    {                             // 20M 0.6G
        BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200104); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // …Ë÷√ºƒ¥Ê∆˜3	£∫∏ﬂ∆µ∂ŒVCO ƒ¨»œ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // …Ë÷√ºƒ¥Ê∆˜2£∫MUXOUT:ƒ¨»œ µÕœ‡‘Î ƒ¨»œ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	…Ë÷√ºƒ¥Ê∆˜1 £∫‘§∑÷∆µ∆˜…ËŒ™4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // …Ë÷√ºƒ¥Ê∆˜0£∫3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 250)
    {                             // 20M 2.5G
        BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200101); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // …Ë÷√ºƒ¥Ê∆˜3	£∫∏ﬂ∆µ∂ŒVCO ƒ¨»œ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // …Ë÷√ºƒ¥Ê∆˜2£∫MUXOUT:ƒ¨»œ µÕœ‡‘Î ƒ¨»œ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	…Ë÷√ºƒ¥Ê∆˜1 £∫‘§∑÷∆µ∆˜…ËŒ™4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // …Ë÷√ºƒ¥Ê∆˜0£∫3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 160)
    {                             // 25M 2G
        BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200101); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // …Ë÷√ºƒ¥Ê∆˜3	£∫∏ﬂ∆µ∂ŒVCO ƒ¨»œ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // …Ë÷√ºƒ¥Ê∆˜2£∫MUXOUT:ƒ¨»œ µÕœ‡‘Î ƒ¨»œ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	…Ë÷√ºƒ¥Ê∆˜1 £∫‘§∑÷∆µ∆˜…ËŒ™4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // …Ë÷√ºƒ¥Ê∆˜0£∫3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    // else if (F == 100)
    // {                             // 40M 2G
    //     BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
    //     usleep(1000);
    //     BR9177_Wdata(0x86200101); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 1110 0010 0000 0000 0000 1000 0010
    //     usleep(1000);
    //     BR9177_Wdata(0x6075c800); // …Ë÷√ºƒ¥Ê∆˜3	£∫∏ﬂ∆µ∂ŒVCO ƒ¨»œ             0110 0001 0111 0101 1100 1000 0
    //     usleep(1000);
    //     BR9177_Wdata(0x43000000); // …Ë÷√ºƒ¥Ê∆˜2£∫MUXOUT:ƒ¨»œ µÕœ‡‘Î ƒ¨»œ       0100 0011 0000
    //     usleep(1000);
    //     BR9177_Wdata(0x29000000); //	…Ë÷√ºƒ¥Ê∆˜1 £∫‘§∑÷∆µ∆˜…ËŒ™4/5 + FRAC 0    0010 1001 0
    //     usleep(1000);
    //     BR9177_Wdata(0x05200000 | F); // …Ë÷√ºƒ¥Ê∆˜0£∫3200uA+INT                 0000 0101 1000 0
    //     usleep(1000);
    // }
    else if (F == 150)
    {                             // 40M 1.5G
        BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // …Ë÷√ºƒ¥Ê∆˜3	£∫∏ﬂ∆µ∂ŒVCO ƒ¨»œ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // …Ë÷√ºƒ¥Ê∆˜2£∫MUXOUT:ƒ¨»œ µÕœ‡‘Î ƒ¨»œ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	…Ë÷√ºƒ¥Ê∆˜1 £∫‘§∑÷∆µ∆˜…ËŒ™4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // …Ë÷√ºƒ¥Ê∆˜0£∫3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 100)
    {                             // 40M 1G
        BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // …Ë÷√ºƒ¥Ê∆˜3	£∫∏ﬂ∆µ∂ŒVCO ƒ¨»œ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // …Ë÷√ºƒ¥Ê∆˜2£∫MUXOUT:ƒ¨»œ µÕœ‡‘Î ƒ¨»œ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	…Ë÷√ºƒ¥Ê∆˜1 £∫‘§∑÷∆µ∆˜…ËŒ™4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // …Ë÷√ºƒ¥Ê∆˜0£∫3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 40)
    {                             // 50M 1G
        BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // …Ë÷√ºƒ¥Ê∆˜3	£∫∏ﬂ∆µ∂ŒVCO ƒ¨»œ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // …Ë÷√ºƒ¥Ê∆˜2£∫MUXOUT:ƒ¨»œ µÕœ‡‘Î ƒ¨»œ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	…Ë÷√ºƒ¥Ê∆˜1 £∫‘§∑÷∆µ∆˜…ËŒ™4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // …Ë÷√ºƒ¥Ê∆˜0£∫3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }

    ESP_LOGI(TAG, "Now Frequency is %d M:", F);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
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
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
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

    GPIO_INIT();
    ESP_LOGI(TAG, "I am here1");
    // BR4351_Init(F);
    ESP_LOGI(TAG, "I am here2");
    BR_CE_Set;
    usleep(1000);

    BR_LE_Set;  //œ»±£÷§CS∆¨—°¿≠∏ﬂ
    BR_CLK_Clr; //œ»±£÷§ ±÷”¿≠µÕ£®…œ…˝—ÿ≤…—˘
    // BR_CLK_Set;//œ»±£÷§ ±÷”¿≠∏ﬂ£®œ¬Ωµ—ÿ≤…—˘
    BR_DATA_Clr;

    int cnt = 0;
    usleep(1000);
    ESP_LOGI(TAG, "Start Set Freq:");

    SetFreq(F);
    ESP_LOGI(TAG, "Freq set.");

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();



    
    
    // esp_netif_get_ip_info(ap_netif, &ip_info);
    // ESP_LOGI(TAG, "THis AP has got an IP as:%s", ip4addr_ntoa((ip4_addr_t*)&ip_info.ip));

}
