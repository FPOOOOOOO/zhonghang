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

#define GPIO_OUTPUT_IO_0 17 // SPI?_CLK（IO32） SPI_MOSI（IO2）已经上拉  SPINSS(IO16)  
#define GPIO_OUTPUT_IO_1 16  // NSS
#define GPIO_OUTPUT_IO_2 2  // MOSI
#define GPIO_OUTPUT_IO_3 32  // CLK
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

#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

#define BUF_SIZE (1024)
#define UART_BUFSIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
static QueueHandle_t uart0_queue;
static QueueHandle_t uart1_queue;

// The semaphore indicating the slave is ready to receive stuff.
static xQueueHandle rdySem;
static const char *TAG = "ADF4351";
static xQueueHandle gpio_evt_queue = NULL;
uint32_t R = 125; // R为参考分配器的数值，计算公式：输入频率/（2*R）=0.1
                  // 注：输入频率的单位为MHz，R的范围为0~1023的整数
                  // 默认输入频率为板载25M晶振，故得R为125。
                  // 板载40M晶振，则R=200
uint32_t F = 30000; // 350 初始频率35MHz

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
    //xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

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
    ADF_LE_Clr;//让32位移位寄存器准备接受下一次数据
    for (i = 0; i < 32; i++)
    {
        if (dat & 0x80000000)
            ADF_DATA_Set;
        else
            ADF_DATA_Clr;
        dat <<= 1;
        ADF_CLK_Set;
        ADF_CLK_Clr;//数据在CLK上升沿时逐个输入32位移位寄存器
    }
    ADF_LE_Set;//打高使得32位移位寄存器按?的寄存器地址进行写入
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
    ESP_LOGI(TAG, "Now Frequency is %d M:",F);
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
                ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(EX_UART_NUM, uart_recv_data, event.size, portMAX_DELAY);
                ESP_LOGI(TAG, "[DATA EVT]:");
                uart_write_bytes(EX_UART_NUM, (const char *)uart_recv_data, event.size);
                SetFreq(F);
                //SetFreq((uint32_t *)uart_recv_data);
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
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 12, NULL);

    GPIO_INIT();
    ESP_LOGI(TAG, "I am here1");
    ADF4351_Init(F);
    ESP_LOGI(TAG, "I am here2");
    int cnt = 0;
    SetFreq(F);
    ESP_LOGI(TAG, "I am here4");

    // while (1)
    // {
    //     printf("cnt: %d\n", cnt++);
    //     vTaskDelay(2000 / portTICK_RATE_MS);
    //     gpio_set_level(GPIO_OUTPUT_IO_0, cnt % 2);
    //     gpio_set_level(GPIO_OUTPUT_IO_1, cnt % 2);
    // }
}
