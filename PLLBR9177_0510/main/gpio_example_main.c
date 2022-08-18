/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <unistd.h>

#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <sys/select.h>
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
/**
 * Brief:
 * This test code shows how to configure gpio and how to use gpio interrupt.
 *
 * GPIO status:
 * GPIO18: output
 * GPIO19: output
 * GPIO4:  input, pulled up, interrupt from rising edge and falling edge
 * GPIO5:  input, pulled up, interrupt from rising edge.
 *
 * Test:
 * Connect GPIO18 with GPIO4
 * Connect GPIO19 with GPIO5
 * Generate pulses on GPIO18/19, that triggers interrupt on GPIO4/5
 *
 */

#define GPIO_OUTPUT_IO_0 25 // SPI?_CLK£¨IO32£© SPI_MOSI£¨IO2£©  SPINSS(IO16)
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
static const char *TAG = "BR4351";
static xQueueHandle gpio_evt_queue = NULL;
uint32_t FREF_DIV = 500; // ÊäÈëÆµÂÊ(MHz)/£¨FREF_DIV£©=0.1(MHz)
uint8_t VCO_DIV = 1;
bool VCO = 0;
uint16_t F = 200; // 120¡ª¡ª1500M 112¡ª¡ª1400M 104¡ª¡ª1300M£¨25M¾§Õñ
                  // 300 3G 250 2.5G 200 2G 300 1.5G 220 1.1G 200 1G 240 0.6G £¨20M¾§Õñ
                  // 100 1G  150  1.5G  100 2G £¨40M¾§Õñ

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
    BR_LE_Clr; //ÈÃ32Î»ÒÆÎ»¼Ä´æÆ÷×¼±¸½ÓÊÜÏÂÒ»´ÎÊý¾Ý
    usleep(15);
    for (i = 0; i < 32; i++)
    {
        if (dat & 0x80000000)
            BR_DATA_Set;
        else
            BR_DATA_Clr;

        BR_CLK_Clr; //ÉÏÉýÑØ²ÉÑù
        usleep(15);
        dat <<= 1;
        BR_CLK_Set;
        usleep(15);

        // BR_CLK_Set; //ÏÂ½µÑØ²ÉÑù
        // usleep(15);
        // dat <<= 1;
        // BR_CLK_Clr;
        // usleep(15);
    }
    BR_CLK_Clr; //ÉÏÉýÑØ²ÉÑù
    // BR_CLK_Set;//ÏÂ½µÑØ²ÉÑù
    usleep(15);
    BR_LE_Set; //´ò¸ßÊ¹µÃ32Î»ÒÆÎ»¼Ä´æÆ÷°´?µÄ¼Ä´æÆ÷µØÖ·½øÐÐÐ´Èë
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
    // if (F == 200)
    // {                             // 20M 2G
    //     BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
    //     usleep(1000);
    //     BR9177_Wdata(0x86200101); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 0110 0010 0000 0000 0000 1000 0010
    //     usleep(1000);
    //     BR9177_Wdata(0x6075c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
    //     usleep(1000);
    //     BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
    //     usleep(1000);
    //     BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
    //     usleep(1000);
    //     BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
    //     usleep(1000);
    // }
    // else if (F == 300)
    // {                             // 20M 1.5G
    //     BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
    //     usleep(1000);
    //     BR9177_Wdata(0x86200102); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 1110 0010 0000 0000 0000 1000 0010
    //     usleep(1000);
    //     BR9177_Wdata(0x6175c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
    //     usleep(1000);
    //     BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
    //     usleep(1000);
    //     BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
    //     usleep(1000);
    //     BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
    //     usleep(1000);
    // }
    if (F == 220)
    {                             // 20M 1.1G
        BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 200)
    {                             // 20M 1G
        BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 0110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 240)
    {                             // 20M 0.6G
        BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200104); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 250)
    {                             // 20M 2.5G
        BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200101); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 160)
    {                             // 25M 2G
        BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200101); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    // else if (F == 100)
    // {                             // 40M 2G
    //     BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
    //     usleep(1000);
    //     BR9177_Wdata(0x86200101); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 1110 0010 0000 0000 0000 1000 0010
    //     usleep(1000);
    //     BR9177_Wdata(0x6075c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
    //     usleep(1000);
    //     BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
    //     usleep(1000);
    //     BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
    //     usleep(1000);
    //     BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
    //     usleep(1000);
    // }
    else if (F == 150)
    {                             // 40M 1.5G
        BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 100)
    {                             // 40M 1G
        BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 40)
    {                             // 50M 1G
        BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x86200102); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6075c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }
    else if (F == 300)
    {                             // 20M 3G
        BR9177_Wdata(0xa000000a); // ÉèÖÃ¼Ä´æÆ÷5                             1010 0000 0000 0000 0000 0000 0000 1010
        usleep(1000);
        BR9177_Wdata(0x8e200101); // ÉèÖÃ¼Ä´æÆ÷4 : RÔ¤·ÖÆµ=1 RF·ÖÆµÆ÷=2 µÍ¹¦ÂÊ  1000 1110 0010 0000 0000 0000 1000 0010
        usleep(1000);
        BR9177_Wdata(0x6175c800); // ÉèÖÃ¼Ä´æÆ÷3	£º¸ßÆµ¶ÎVCO Ä¬ÈÏ             0110 0001 0111 0101 1100 1000 0
        usleep(1000);
        BR9177_Wdata(0x43000000); // ÉèÖÃ¼Ä´æÆ÷2£ºMUXOUT:Ä¬ÈÏ µÍÏàÔë Ä¬ÈÏ       0100 0011 0000
        usleep(1000);
        BR9177_Wdata(0x29000000); //	ÉèÖÃ¼Ä´æÆ÷1 £ºÔ¤·ÖÆµÆ÷ÉèÎª4/5 + FRAC 0    0010 1001 0
        usleep(1000);
        BR9177_Wdata(0x05200000 | F); // ÉèÖÃ¼Ä´æÆ÷0£º3200uA+INT                 0000 0101 1000 0
        usleep(1000);
    }

    ESP_LOGI(TAG, "Now Frequency is %d M:", F);
}

static void uart_task(void *pvParameters)
{
    //????UART
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
    uint8_t *uart_recv_data = (uint8_t *)malloc(UART_BUFSIZE + 1);

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
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                const int rxBytes = uart_read_bytes(EX_UART_NUM, uart_recv_data, event.size, portMAX_DELAY);
                ESP_LOGI(TAG, "[DATA EVT]:");
                uart_write_bytes(EX_UART_NUM, (const char *)uart_recv_data, event.size);
                // memcpy(&uart_data, uart_recv_data, 2);
                // FF=*uart_recv_data;
                ESP_LOGI(TAG, "uart_data is %s:", uart_recv_data);
                uart_data = atoi((const char *)uart_recv_data);
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

void app_main(void)
{

    GPIO_INIT();
    ESP_LOGI(TAG, "I am here1");
    // BR4351_Init(F);
    ESP_LOGI(TAG, "I am here2");
    BR_CE_Set;
    usleep(1000);

    BR_LE_Set;  //ÏÈ±£Ö¤CSÆ¬Ñ¡À­¸ß
    BR_CLK_Clr; //ÏÈ±£Ö¤Ê±ÖÓÀ­µÍ£¨ÉÏÉýÑØ²ÉÑù
    // BR_CLK_Set;//ÏÈ±£Ö¤Ê±ÖÓÀ­¸ß£¨ÏÂ½µÑØ²ÉÑù
    BR_DATA_Clr;

    int cnt = 0;
    usleep(1000);
    ESP_LOGI(TAG, "Start Set Freq:");

    SetFreq(F);
    ESP_LOGI(TAG, "I am here4");
    // usleep(1000);
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 12, NULL);
    // while (1)
    // {
    //     printf("cnt: %d\n", cnt++);
    //     vTaskDelay(2000 / portTICK_RATE_MS);
    //     gpio_set_level(GPIO_OUTPUT_IO_0, cnt % 2);
    //     gpio_set_level(GPIO_OUTPUT_IO_1, cnt % 2);
    // }
}
