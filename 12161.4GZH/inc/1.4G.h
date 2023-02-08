//1216 FPO
#include "mdf_common.h"
#include "mwifi.h"
#include "driver/uart.h"

#define GPIO_OUTPUT_IO_0 25 // SPI?_CLK£¨IO32£© SPI_MOSI£¨IO2£©  SPINSS(IO16)
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

void GPIO_INIT(void);

void ADF4351_Init(uint32_t date);
void ADF4351_Wdata(uint32_t dat);

void SetFreq(uint32_t F);

extern uint32_t F;