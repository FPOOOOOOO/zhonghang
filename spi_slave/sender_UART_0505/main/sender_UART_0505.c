/* SPI Slave example, sender (uses SPI master driver)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/igmp.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "soc/rtc_periph.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_spi_flash.h"

#include "driver/gpio.h"
#include "esp_intr_alloc.h"

#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_timer.h"

/*
SPI sender (master) example.

This example is supposed to work together with the SPI receiver. It uses the standard SPI pins (MISO, MOSI, SCLK, CS) to
transmit data over in a full-duplex fashion, that is, while the master puts data on the MOSI pin, the slave puts its own
data on the MISO pin.

This example uses one extra pin: GPIO_HANDSHAKE is used as a handshake pin. The slave makes this pin high as soon as it is
ready to receive/send data. This code connects this line to a GPIO interrupt which gives the rdySem semaphore. The main
task waits for this semaphore to be given before queueing a transmission.
*/

/*
Pins in use. The SPI Master can use the GPIO mux, so feel free to change these if needed.
*/

static const char *TAG = "sender_UART_0505";

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define GPIO_HANDSHAKE 0 // 2default added
#define GPIO_MOSI 13     // 12
#define GPIO_MISO 12     // 13
#define GPIO_SCLK 14     // 15
#define GPIO_CS 15       // 14

#elif CONFIG_IDF_TARGET_ESP32C3
#define GPIO_HANDSHAKE 3
#define GPIO_MOSI 7
#define GPIO_MISO 2
#define GPIO_SCLK 6
#define GPIO_CS 10


#endif // CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2

#ifdef CONFIG_IDF_TARGET_ESP32
#define SENDER_HOST HSPI_HOST

#elif defined CONFIG_IDF_TARGET_ESP32S2
#define SENDER_HOST SPI2_HOST

#elif defined CONFIG_IDF_TARGET_ESP32C3
#define SENDER_HOST SPI2_HOST

#endif

#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/

//added
#define TXD1_PIN (GPIO_NUM_33) //??1???????
#define RXD1_PIN (GPIO_NUM_35) //??1???????


#define BUF_SIZE (1024)
#define UART_BUFSIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
static QueueHandle_t uart0_queue;
static QueueHandle_t uart1_queue;

// The semaphore indicating the slave is ready to receive stuff.
static xQueueHandle rdySem;

/*
This ISR is called when the handshake line goes high.
*/
// static void IRAM_ATTR gpio_handshake_isr_handler(void* arg)
// {
//     //Sometimes due to interference or ringing or something, we get two irqs after eachother. This is solved by
//     //looking at the time between interrupts and refusing any interrupt too close to another one.
//     static uint32_t lasthandshaketime;
//     uint32_t currtime=esp_cpu_get_ccount();
//     uint32_t diff=currtime-lasthandshaketime;
//     if (diff<240000) return; //ignore everything <1ms after an earlier irq
//     lasthandshaketime=currtime;

//     //Give the semaphore.
//     BaseType_t mustYield=false;
//     xSemaphoreGiveFromISR(rdySem, &mustYield);
//     if (mustYield) portYIELD_FROM_ISR();
// }

void pre_cb(transaction_cb_t *trans)
{
    BaseType_t mustYield = true;
    // xSemaphoreGiveFromISR(rdySem, &mustYield);
}

void post_cb(transaction_cb_t *trans)
{
    BaseType_t mustYield = false;
    // xSemaphoreGiveFromISR(rdySem, &mustYield);
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
                xSemaphoreGive(rdySem);

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

static void spi_task(void *pvParameters){
    esp_err_t ret;
    spi_device_handle_t handle;

    // Configuration for the SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .flags = SPICOMMON_BUSFLAG_IOMUX_PINS, // added IOMUX
        .quadwp_io_num = -1,                   // added -1 default
        .quadhd_io_num = -1                    //
    };
    printf("buscfg¡®s flag is::: %u\n", buscfg.flags);
    // Configuration for the SPI device on the other side of the bus
    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .clock_speed_hz = 4000000, // 5000000 at initial added
        .duty_cycle_pos = 128,     // 50% duty cycle
        .mode = 0,
        //.input_delay_ns=1,          //default null,added
        .spics_io_num = GPIO_CS,
        .cs_ena_posttrans = 3, // Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
        .queue_size = 3      // 3 defalut added
        //.pre_cb = pre_cb,
        //.post_cb = post_cb
        };

    // GPIO config for the handshake line.
    //  gpio_config_t io_conf={
    //      .intr_type=GPIO_INTR_POSEDGE,
    //      .mode=GPIO_MODE_INPUT,
    //      .pull_up_en=1,
    //      .pin_bit_mask=(1<<GPIO_HANDSHAKE)
    //  };

    int n = 0;
    char sendbuf[128] = {0};
    char recvbuf[128] = {0};
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    // Create the semaphore.
    rdySem = xSemaphoreCreateBinary();

    // Set up handshake line interrupt.
    //  gpio_config(&io_conf);
    //  gpio_install_isr_service(0);
    //  gpio_set_intr_type(GPIO_HANDSHAKE, GPIO_INTR_POSEDGE);
    //  gpio_isr_handler_add(GPIO_HANDSHAKE, gpio_handshake_isr_handler, NULL);

    // Initialize the SPI bus and add the device we want to send stuff to.
    ret = spi_bus_initialize(SENDER_HOST, &buscfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);
    ret = spi_bus_add_device(SENDER_HOST, &devcfg, &handle);
    assert(ret == ESP_OK);

    // Assume the slave is ready for the first transmission: if the slave started up before us, we will not detect
    // positive edge on the handshake line.
    xSemaphoreGive(rdySem);

    while (1)
    {
        memset(recvbuf, 0xB5, 128);

        // int res = snprintf(sendbuf, sizeof(sendbuf),
        //         "Sender, transmission no. %04i. Last time, I received: \"%s\"", n, recvbuf);
        int res = snprintf(sendbuf, sizeof(sendbuf),
                           "Sender, transmission no. %04i", n);
        printf("Res Length is %d\n", res);
        if (res >= sizeof(sendbuf))
        {
            printf("Data truncated\n");
        }
        t.length = sizeof(sendbuf) * 8;
        t.tx_buffer = sendbuf;
        t.rx_buffer = recvbuf;
        // Wait for slave to be ready for next byte before sending
        xSemaphoreTake(rdySem, portMAX_DELAY); //Wait until slave is ready
        ret = spi_device_transmit(handle, &t); //   Equals spi_device_queue_trans() +spi_device_get_trans_results
        // ret = spi_device_queue_trans(handle, t, portMAX_DELAY);
        // if (ret != ESP_OK) return ret;
        printf("Received: %s\n", recvbuf);
        vTaskDelay(pdMS_TO_TICKS(1000));
        n++;
        if (n == 100)
        {
            printf("Already done : %d\n", n);
            n=0;
        }
    }

    // Never reached.
    ret = spi_bus_remove_device(handle);
    assert(ret == ESP_OK);

}
// Main application
void app_main(void)
{
    xTaskCreate(spi_task, "spi_task", 2048, NULL, 12, NULL);
    xTaskCreate(uart_task, "uart_task", 2048, NULL, 12, NULL);
}
