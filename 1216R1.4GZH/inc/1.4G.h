//1216 FPO
#include "mdf_common.h"
#include "mwifi.h"
#include "driver/uart.h"

void GPIO_INIT(void);

void ADF4351_Init(uint32_t date);
void ADF4351_Wdata(uint32_t dat);

void SetFreq(uint32_t F);

extern uint32_t F;