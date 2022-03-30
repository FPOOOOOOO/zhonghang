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
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define GPIO_HANDSHAKE 0 //2default added
#define GPIO_MOSI 13     //12
#define GPIO_MISO 12     //13
#define GPIO_SCLK 14     //15
#define GPIO_CS 15       //14

#elif CONFIG_IDF_TARGET_ESP32C3
#define GPIO_HANDSHAKE 3
#define GPIO_MOSI 7
#define GPIO_MISO 2
#define GPIO_SCLK 6
#define GPIO_CS 10

#endif //CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2


#ifdef CONFIG_IDF_TARGET_ESP32
#define SENDER_HOST HSPI_HOST

#elif defined CONFIG_IDF_TARGET_ESP32S2
#define SENDER_HOST SPI2_HOST

#elif defined CONFIG_IDF_TARGET_ESP32C3
#define SENDER_HOST SPI2_HOST

#endif


//The semaphore indicating the slave is ready to receive stuff.
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

void pre_cb(transaction_cb_t *trans) {
    BaseType_t mustYield=true;
    //xSemaphoreGiveFromISR(rdySem, &mustYield);
}

void post_cb(transaction_cb_t *trans) {
    BaseType_t mustYield=false;
    //xSemaphoreGiveFromISR(rdySem, &mustYield);
}

//Main application
void app_main(void)
{
    esp_err_t ret;
    spi_device_handle_t handle;

    //Configuration for the SPI bus
    spi_bus_config_t buscfg={
        .mosi_io_num=GPIO_MOSI,
        .miso_io_num=GPIO_MISO,
        .sclk_io_num=GPIO_SCLK,
        .flags=SPICOMMON_BUSFLAG_IOMUX_PINS, //added IOMUX
        .quadwp_io_num=-1,  //added -1 default
        .quadhd_io_num=-1   //
    };
     printf("buscfg��s flag is::: %u\n",buscfg.flags);
    //Configuration for the SPI device on the other side of the bus
    spi_device_interface_config_t devcfg={
        .command_bits=0,
        .address_bits=0,
        .dummy_bits=0,
        .clock_speed_hz=8000000,    //5000000 at initial added
        .duty_cycle_pos=128,        //50% duty cycle
        .mode=0,
        //.input_delay_ns=1,          //default null,added
        .spics_io_num=GPIO_CS,
        .cs_ena_posttrans=3,        //Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
        .queue_size=3,               //3 defalut added
        .pre_cb=pre_cb,
        .post_cb=post_cb
    };

    //GPIO config for the handshake line.
    // gpio_config_t io_conf={
    //     .intr_type=GPIO_INTR_POSEDGE,
    //     .mode=GPIO_MODE_INPUT,
    //     .pull_up_en=1,
    //     .pin_bit_mask=(1<<GPIO_HANDSHAKE)
    // };

    int n=0;
    char sendbuf[128] = {0};
    char recvbuf[128] = {0};
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));

    //Create the semaphore.
    rdySem=xSemaphoreCreateBinary();

    //Set up handshake line interrupt.
    // gpio_config(&io_conf);
    // gpio_install_isr_service(0);
    // gpio_set_intr_type(GPIO_HANDSHAKE, GPIO_INTR_POSEDGE);
    // gpio_isr_handler_add(GPIO_HANDSHAKE, gpio_handshake_isr_handler, NULL);

    //Initialize the SPI bus and add the device we want to send stuff to.
    ret=spi_bus_initialize(SENDER_HOST, &buscfg, SPI_DMA_CH_AUTO);
    assert(ret==ESP_OK);
    ret=spi_bus_add_device(SENDER_HOST, &devcfg, &handle);
    assert(ret==ESP_OK);

    //Assume the slave is ready for the first transmission: if the slave started up before us, we will not detect
    //positive edge on the handshake line.
    xSemaphoreGive(rdySem);

    while(1) {
        int res = snprintf(sendbuf, sizeof(sendbuf),
                "Sender, transmission no. %04i. Last time, I received: \"%s\"", n, recvbuf);
        if (res >= sizeof(sendbuf)) {
            printf("Data truncated\n");
        }
        t.length=sizeof(sendbuf)*8;
        t.tx_buffer=sendbuf;
        t.rx_buffer=recvbuf;
        //Wait for slave to be ready for next byte before sending
        //xSemaphoreTake(rdySem, portMAX_DELAY); //Wait until slave is ready
        ret=spi_device_transmit(handle, &t);//   Equals spi_device_get_trans_results  +  spi_device_queue_trans()
        //ret = spi_device_queue_trans(handle, t, portMAX_DELAY);
        //if (ret != ESP_OK) return ret;
        printf("Received: %s\n", recvbuf);
        vTaskDelay(pdMS_TO_TICKS(10));
        n++;
        // if(n==100){
        //     printf("Already done : %d\n",n);
        //     break;
        // }
    }

    //Never reached.
    ret=spi_bus_remove_device(handle);
    assert(ret==ESP_OK);
}
 