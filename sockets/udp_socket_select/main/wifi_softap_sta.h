#ifndef wifi_softap_h_
#define wifi_softap_h_

#ifndef wifi_softap_c_
#define wifi_softap_cx_ extern
#else
#define wifi_softap_cx_
#endif

#include "event_handler.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"


wifi_softap_cx_ EventGroupHandle_t EventGroupHandleWiFiEvent;

#define EventBitsWiFiStaConnected BIT0

void wifi_init_softap_sta(void);



#endif