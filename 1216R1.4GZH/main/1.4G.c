//1216 FPO
#include "1.4G.h"

#define GPIO_OUTPUT_IO_0 25 // SPI?_CLK（IO32） SPI_MOSI（IO2）  SPINSS(IO16)
#define GPIO_OUTPUT_IO_1 16 // NSS
#define GPIO_OUTPUT_IO_2 2  // MOSI
#define GPIO_OUTPUT_IO_3 32 // CLK

#define GPIO_OUTPUT_8201 17 //8201 boot later

#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1) | (1ULL << GPIO_OUTPUT_IO_2) | (1ULL << GPIO_OUTPUT_IO_3)| (1ULL << GPIO_OUTPUT_8201))
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

#define SR8201_H (gpio_set_level(GPIO_OUTPUT_8201, 1))
#define SR8201_L (gpio_set_level(GPIO_OUTPUT_8201, 0))

static xQueueHandle gpio_evt_queue = NULL;
uint32_t R = 100;   // R为参考分配器的数值，计算公式：输入频率/（2*R）=0.1
                    // 注：输入频率的单位为MHz，R的范围为0~1023的整数
                    // 默认输入频率为板载25M晶振，故得R为125。
                    // 板载40M晶振，则R=200
                    
uint32_t F = 39000; // 38000 初始频率3800MHz

static const char *TAG = "1.4G";

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

void GPIO_INIT(void)
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
