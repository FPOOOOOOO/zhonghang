#define usart_c_

#include "usart.h"
#include "main.c"

static const char *TAG = "usart_function";



#define TXD1_PIN (GPIO_NUM_17) //串口1的发送数据引脚
#define RXD1_PIN (GPIO_NUM_16) //串口1的接收数据引脚

#define USART1_BUF_SIZE_RECV (1024) //接收数据缓存大小,该大小需要大于内部FIFO大小:UART_FIFO_LEN(128)
#define USART1_BUF_SIZE_SEND (1024) //接收数据缓存大小,该大小需要大于内部FIFO大小:UART_FIFO_LEN(128)

uint8_t rb_t_usart1_read_buff[USART1_BUF_SIZE_RECV];


/*串口发送数据*/
void usart_send(uint8_t *data ,int len)
{
    uart_write_bytes(UART_NUM_1, (const char *) data, len);
}


/*串口任务*/
static void uart_task(void *arg)
{
    int usart1_read_count=0;//接收计数
    int usart1_idle_count=0;//接收计数
    /*配置串口参数*/
    uart_config_t uart_config = {
        .baud_rate = 115200,//波特率
        .data_bits = UART_DATA_8_BITS,//数据位8位
        .parity    = UART_PARITY_DISABLE,//无奇偶校验
        .stop_bits = UART_STOP_BITS_1,//停止位1位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,//不使用硬件流控
        .source_clk = UART_SCLK_APB,//串口使用的时钟
    };
    /*初始化串口1*/
    uart_driver_install(UART_NUM_1,
        USART1_BUF_SIZE_RECV, //串口1接收缓存大小
        1024,
        0, //队列大小为0;没有使用freertos内部缓存管理
        NULL, //不使用QueueHandle_t 内部缓存管理,设置为空
        0 //设置串口中断优先级,设置为0意味着让系统从1-3级中自动选择一个
    );
    /*设置串口参数*/
    uart_param_config(UART_NUM_1, &uart_config);
    /*设置串口的TX,RX,RTS,DTR引脚*/             //不使用RTS,DTR
    uart_set_pin(UART_NUM_1, TXD1_PIN, RXD1_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    /*申请一块内存,用于临时存储接收的数据*/
    uint8_t *uart_recv_data = (uint8_t *) malloc(USART1_BUF_SIZE_RECV);
    while (1) {
        //接收串口数据                                         //每隔20ms判断一次,可以写成portMAX_DELAY(一直判断)
        int len = uart_read_bytes(UART_NUM_1, uart_recv_data, USART1_BUF_SIZE_RECV, 20 / portTICK_RATE_MS);
        
        if(len>0)//接收到数据,把数据存储到缓存
        {
            usart1_idle_count=0;
            memcpy(rb_t_usart1_read_buff+usart1_read_count,uart_recv_data,len);
            usart1_read_count = usart1_read_count + len;
        }
        else//没有数据
        {
            if(usart1_read_count!=0)//已经接收到数据
            {
                usart1_idle_count ++;
                if(usart1_idle_count>=2)
                {
                    usart1_idle_count=0;
                    /*处理接收的数据*/
                    //串口接收到的数据:rb_t_usart1_read_buff  数据长度:usart1_read_count
                    

                    rb_t_usart1_read_buff[usart1_read_count]=0;
                    usart1_read_count=0;
                }
            }
        }
    }
    free(uart_recv_data);
}



void usart_init(int usart_task_priority)
{
    xTaskCreate(uart_task, "uart_task", 4096, NULL, usart_task_priority, NULL);
}

