/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
   0719:‘ˆº”√ø¥ŒRSSIªÒ»°
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

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

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY

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

#define GPIOTIME 15

static QueueHandle_t uart0_queue;
static QueueHandle_t uart1_queue;

// The semaphore indicating the slave is ready to receive stuff.
static uint16_t uart_data = 0x0000;
static xQueueHandle rdySem;
static xQueueHandle gpio_evt_queue = NULL;
uint32_t FREF_DIV = 500; //  ‰»Î∆µ¬ (MHz)/£®FREF_DIV£©=0.1(MHz)
uint8_t VCO_DIV = 1;
bool VCO = 0;
uint16_t F = 225; // 120°™°™1500M 112°™°™1400M 104°™°™1300M£®25Mæß’Ò
                  // 250 2.5G 200 2G 300 1.5G 220 1.1G 240 0.6G £®20Mæß’Ò
                  // 100 1G  150  1.5G  100 2G £®40Mæß’Ò

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;

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
    usleep(GPIOTIME);
    for (i = 0; i < 32; i++)
    {
        if (dat & 0x80000000)
            BR_DATA_Set;
        else
            BR_DATA_Clr;

        BR_CLK_Clr; //…œ…˝—ÿ≤…—˘
        usleep(GPIOTIME);
        dat <<= 1;
        BR_CLK_Set;
        usleep(GPIOTIME);

        // BR_CLK_Set; //œ¬Ωµ—ÿ≤…—˘
        // usleep(15);
        // dat <<= 1;
        // BR_CLK_Clr;
        // usleep(15);
    }
    BR_CLK_Clr; //…œ…˝—ÿ≤…—˘
    // BR_CLK_Set;//œ¬Ωµ—ÿ≤…—˘
    usleep(GPIOTIME);
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
    // else if (F == 240)
    // {                             // 20M 0.6G
    //     BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
    //     usleep(1000);
    //     BR9177_Wdata(0x86200104); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 1110 0010 0000 0000 0000 1000 0010
    //     usleep(1000);
    //     BR9177_Wdata(0x6175c800); // …Ë÷√ºƒ¥Ê∆˜3	£∫∏ﬂ∆µ∂ŒVCO ƒ¨»œ             0110 0001 0111 0101 1100 1000 0
    //     usleep(1000);
    //     BR9177_Wdata(0x43000000); // …Ë÷√ºƒ¥Ê∆˜2£∫MUXOUT:ƒ¨»œ µÕœ‡‘Î ƒ¨»œ       0100 0011 0000
    //     usleep(1000);
    //     BR9177_Wdata(0x29000000); //	…Ë÷√ºƒ¥Ê∆˜1 £∫‘§∑÷∆µ∆˜…ËŒ™4/5 + FRAC 0    0010 1001 0
    //     usleep(1000);
    //     BR9177_Wdata(0x05200000 | F); // …Ë÷√ºƒ¥Ê∆˜0£∫3200uA+INT                 0000 0101 1000 0
    //     usleep(1000);
    // }
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
    else if (F == 100)
    {                             // 40M 2G
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
    // else if (F == 100)
    // {                             // 40M 1G
    //     BR9177_Wdata(0xa000000a); // …Ë÷√ºƒ¥Ê∆˜5                             1010 0000 0000 0000 0000 0000 0000 1010
    //     usleep(1000);
    //     BR9177_Wdata(0x86200102); // …Ë÷√ºƒ¥Ê∆˜4 : R‘§∑÷∆µ=1 RF∑÷∆µ∆˜=2 µÕπ¶¬   1000 1110 0010 0000 0000 0000 1000 0010
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
    else if (F == 225)
    {                             // 40M 3G
        BR9177_Wdata(0xa000000a); // ?????5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200181); // ?????4 : R???=1 RF???=2 ???  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // ?????3	????VCO ??             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ?????2?MUXOUT:?? ??? ??       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	?????1 ???????4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ?????0?3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
        else if (F == 240)
    {                             // 40M 3.2G
        BR9177_Wdata(0xa000000a); // ?????5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200181); // ?????4 : R???=1 RF???=2 ???  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // ?????3	????VCO ??             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ?????2?MUXOUT:?? ??? ??       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	?????1 ???????4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ?????0?3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
            else if (F == 297)
    {                             // 40M 2.99G
        BR9177_Wdata(0xa000000a); // ?????5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200201); // ?????4 : R???=1 RF???=2 ???  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // ?????3	????VCO ??             0110 0001 0111 0101 1100 1000 0
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

// Forward packets from Wi-Fi to Ethernet
static esp_err_t pkt_wifi2sta(void *buffer, uint16_t len, void *eb)
{
    wifi_ap_record_t ap_info;
    esp_wifi_sta_get_ap_info(&ap_info);
    ESP_LOGI(TAG, "Here is %s =: %d , RSSI = %d ", (char *)buffer, len, ap_info.rssi);
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
        ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED\r\n");
        wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
        ESP_LOGI(TAG, "AP " MACSTR " HAS CONNECTED", MAC2STR(event->bssid));
        // example_set_static_ip(arg);
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        esp_wifi_internal_reg_rxcb(WIFI_IF_STA, pkt_wifi2sta);
    }
    // else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    //     ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    //     ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    //     s_retry_num = 0;
    //     xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    // }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

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

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
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
    SetFreq(F);
    SetFreq(F);
    ESP_LOGI(TAG, "Freq set.");

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
}
