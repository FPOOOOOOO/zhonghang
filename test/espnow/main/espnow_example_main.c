/* ESPNOW Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


/*
   This example shows how to use ESPNOW.
   Prepare two device, one for sending ESPNOW data and another for receiving
   ESPNOW data.
*/
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "espnow_example.h"

#define ESPNOW_MAXDELAY 512//在xQueensend里面会用到

static const char *TAG = "espnow_example";//打印调试的时候用到可以改

static xQueueHandle s_example_espnow_queue;

static uint8_t s_example_broadcast_mac[ESP_NOW_ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };//默认的地址，用来广播用，但是一般不会这么用
/*自己添加的两个mac地址*/
static uint8_t esp32_mac1[ESP_NOW_ETH_ALEN] = { 0xC4, 0xDD, 0x57, 0xCA, 0xBC, 0x30 };
static uint8_t esp32_mac2[ESP_NOW_ETH_ALEN] = { 0xC4, 0xDD, 0x57, 0xCA, 0x99, 0x00 };
/* 用来给broad和uni计数，芜湖，大屁忒牛辣 */
static uint16_t s_example_espnow_seq[EXAMPLE_ESPNOW_DATA_MAX] = { 0, 0 };

static void example_espnow_deinit(example_espnow_send_param_t *send_param);

/* WiFi should start before using ESPNOW */
static void example_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());//初始化底层TCP/IP堆栈
    ESP_ERROR_CHECK(esp_event_loop_create_default());//注册事件处理函数初始化
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();//获取wifi初始化配置
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );//设置wifi配置
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );//设置存储类别
    ESP_ERROR_CHECK( esp_wifi_set_mode(ESPNOW_WIFI_MODE) );//设置wifi类别，default soft AP
    ESP_ERROR_CHECK( esp_wifi_start());


#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK( esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
#endif
}

/* 创建一个evt，然后在把send_cb指向evt.info.send_cb，设置id、把参数mac、status赋值到send_cb
 ESPNOW sending or receiving callback function is called in WiFi task.
 * Users should not do lengthy operations from this task. Instead, post
 * necessary data to a queue and handle it from a lower priority task. */
static void example_espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    example_espnow_event_t evt;
    example_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;

    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Send cb arg error");
        return;
    }

    evt.id = EXAMPLE_ESPNOW_SEND_CB;
    memcpy(send_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    send_cb->status = status;
    if (xQueueSend(s_example_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send send queue fail");
    }
}

static void example_espnow_recv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    example_espnow_event_t evt;
    example_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

    if (mac_addr == NULL || data == NULL || len <= 0) {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    evt.id = EXAMPLE_ESPNOW_RECV_CB;
    memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    recv_cb->data = malloc(len);
    if (recv_cb->data == NULL) {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }
    memcpy(recv_cb->data, data, len);
    recv_cb->data_len = len;
    if (xQueueSend(s_example_espnow_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE) {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(recv_cb->data);
    }
}

/* Parse received ESPNOW data. Return buf->type 拿接收到的state、seq_num、magic、crc，同时改变crc */
int example_espnow_data_parse(uint8_t *data, uint16_t data_len, uint8_t *state, uint16_t *seq, int *magic,uint8_t *payload)
{
    example_espnow_data_t *buf = (example_espnow_data_t *)data;
    uint16_t crc, crc_cal = 0;

    if (data_len < sizeof(example_espnow_data_t)) {
        ESP_LOGE(TAG, "Receive ESPNOW data too short, len:%d", data_len);
        return -1;
    }

    *state = buf->state;//拿state
    *seq = buf->seq_num;//拿
    *magic = buf->magic;
    *payload =  buf->payload;
    crc = buf->crc;
    buf->crc = 0;//验证后把crc变成
    crc_cal = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, data_len);

    if (crc_cal == crc) {
        return buf->type;
    }

    return -1;
}

/* Prepare ESPNOW data to be sent. */
void example_espnow_data_prepare(example_espnow_send_param_t *send_param)
{
    example_espnow_data_t *buf = (example_espnow_data_t *)send_param->buffer;//作为结构里的指针

    assert(send_param->len >= sizeof(example_espnow_data_t));

    buf->type = IS_BROADCAST_ADDR(send_param->dest_mac) ? EXAMPLE_ESPNOW_DATA_BROADCAST : EXAMPLE_ESPNOW_DATA_UNICAST;//问号前=1就是broad，=0就是uni
    buf->state = send_param->state;
    buf->seq_num = s_example_espnow_seq[buf->type]++;//prepare调用一次，就在在相应的类型下++，牛啊这么细
    buf->crc = 0;
    buf->magic = send_param->magic;
    /* Fill all remaining bytes after the data with random values */ //在这里加我要发的东西
    esp_fill_random(buf->payload, send_param->len - sizeof(example_espnow_data_t));
    buf->crc = esp_crc16_le(UINT16_MAX, (uint8_t const *)buf, send_param->len);
}

static void example_espnow_task(void *pvParameter)
{
    example_espnow_event_t evt;
    uint8_t recv_state = 0;
    uint16_t recv_seq = 0;
    int recv_magic = 0;
    uint8_t recv_payload =0; 
    bool is_broadcast = false;
    int ret;

    vTaskDelay(5000 / portTICK_RATE_MS);
    ESP_LOGI(TAG, "Start sending broadcast data");

    /* Start sending broadcast ESPNOW data. */
    example_espnow_send_param_t *send_param = (example_espnow_send_param_t *)pvParameter;//1、获取发送参数
    if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
//    if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
        ESP_LOGE(TAG, "Send error");
        example_espnow_deinit(send_param);
        vTaskDelete(NULL);
    }//2、开始发送第一次数据，这里才是循环开始的地方，send data to ff:ff:ff:ff:ff:ff[0m

    while (xQueueReceive(s_example_espnow_queue, &evt, portMAX_DELAY) == pdTRUE) {//判断是否有事件，这里应该可以拿到对面的mac
        switch (evt.id) {
            ESP_LOGI(TAG, "evt.it= %d ", evt.id);//debug的时候才会打印
            case EXAMPLE_ESPNOW_SEND_CB:
            {
                ESP_LOGI(TAG, "Enter case SEND");
                example_espnow_event_send_cb_t *send_cb = &evt.info.send_cb;
                //ESP_LOGI(TAG, "Mac "MACSTR", status1: %d", MAC2STR(send_cb->mac_addr), send_cb->status);//debug的时候才会打印
                is_broadcast = IS_BROADCAST_ADDR(send_cb->mac_addr);//把括号内的和s_example_broadcast_mac比较是否相等，1:不等于 0:等于。
               // ESP_LOGI(TAG, "is_broadcast %d", is_broadcast);//debug的时候才会打印
                ESP_LOGD(TAG, "Send data to "MACSTR", status1: %d", MAC2STR(send_cb->mac_addr), send_cb->status);//debug的时候才会打印

                if (is_broadcast && (send_param->broadcast == false)) {
                    break;
                }

                if (!is_broadcast) {
                    send_param->count--;//unicast一次减1，每次减的都是上一次的发送
                    if (send_param->count == 0) {
                        ESP_LOGI(TAG, "Send done");
                        example_espnow_deinit(send_param);
                        vTaskDelete(NULL);
                    }
                }

                /* Delay a while before sending the next data. */
                if (send_param->delay > 0) {
                    vTaskDelay(send_param->delay/portTICK_RATE_MS);
                }

                ESP_LOGI(TAG, "Send %dth data to "MACSTR"", ((example_espnow_data_t *)send_param->buffer)->seq_num, MAC2STR(send_cb->mac_addr));//send data to ff:ff:ff:ff:ff:ff[0m，发送标志的也是代码段上面?的发送
                ESP_LOGI(TAG, "The Data is %c", ((example_espnow_data_t *)send_param->buffer)->payload[0]);
                // ESP_LOGI(TAG, "Receive %dth unicast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);//在这里打印收到的消息

                memcpy(send_param->dest_mac, send_cb->mac_addr, ESP_NOW_ETH_ALEN);//发完一次就把地址复制到send_param中
                example_espnow_data_prepare(send_param);//为下一次发送做data_prepare准备

                /* Send the next data after the previous data is sent. */
                if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {
                    ESP_LOGE(TAG, "Send error");
                    example_espnow_deinit(send_param);
                    vTaskDelete(NULL);
                }
                break;
            }
            case EXAMPLE_ESPNOW_RECV_CB:
            {
                example_espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;
                ESP_LOGI(TAG, "Enter case RECV");
                ret = example_espnow_data_parse(recv_cb->data, recv_cb->data_len, &recv_state, &recv_seq, &recv_magic,&recv_payload);//对收到的数据进行处理，判断是广播还是单播消息
                free(recv_cb->data);
                if (ret == EXAMPLE_ESPNOW_DATA_BROADCAST) {
                    ESP_LOGI(TAG, "Receive %dth broadcast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);//打印的东西
                    ESP_LOGI(TAG, "Data is: %d", recv_payload);//打印的东西
                    /* If MAC address does not exist in peer list, add it to peer list. */
                    if (esp_now_is_peer_exist(recv_cb->mac_addr) == false) {
                        esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
                        if (peer == NULL) {
                            ESP_LOGE(TAG, "Malloc peer information fail");
                            example_espnow_deinit(send_param);
                            vTaskDelete(NULL);
                        }
                        memset(peer, 0, sizeof(esp_now_peer_info_t));
                        peer->channel = CONFIG_ESPNOW_CHANNEL;
                        peer->ifidx = ESPNOW_WIFI_IF;
                        peer->encrypt = true;
                        memcpy(peer->lmk, CONFIG_ESPNOW_LMK, ESP_NOW_KEY_LEN);
                        memcpy(peer->peer_addr, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                        ESP_ERROR_CHECK( esp_now_add_peer(peer) );
                        free(peer);
                    }

                    /* Indicates that the device has received broadcast ESPNOW data. */
                    if (send_param->state == 0) {
                        send_param->state = 1;
                    }

                    /* If receive broadcast ESPNOW data which indicates that the other device has received
                     * broadcast ESPNOW data and the local magic number is bigger than that in the received
                     * broadcast ESPNOW data, stop sending broadcast ESPNOW data and start sending unicast
                     * ESPNOW data.
                     */
                    if (recv_state == 1) {//这里的recv_state是从example_espnow_data_parse中被赋值的，最后是从evt里面拿到的。
                        /* The device which has the bigger magic number sends ESPNOW data, the other one
                         * receives ESPNOW data.//决定收发关系，居然是这样靠magic number，怪不得会随机收发；进到这里说明已经要作为发送端了
                         */
                        if (send_param->unicast == false && send_param->magic >= recv_magic) {
                            ESP_LOGI(TAG, "send_param magic is %d",send_param->magic);//打印神奇数字
                            ESP_LOGI(TAG, "recv_magic is %d",recv_magic);//打印神奇数字
                    	    ESP_LOGI(TAG, "Start sending unicast data");
                    	    ESP_LOGI(TAG, "send unicast data to "MACSTR"", MAC2STR(recv_cb->mac_addr));//这里只显示发了一遍

                    	    /* Start sending unicast ESPNOW data. */
                            memcpy(send_param->dest_mac, recv_cb->mac_addr, ESP_NOW_ETH_ALEN);
                            example_espnow_data_prepare(send_param);
                            if (esp_now_send(send_param->dest_mac, send_param->buffer, send_param->len) != ESP_OK) {//在这里发的一遍，上下关联
                                ESP_LOGE(TAG, "Send error");
                                example_espnow_deinit(send_param);
                                vTaskDelete(NULL);
                            }
                            else {
                                send_param->broadcast = false;
                                send_param->unicast = true;
                            }
                        }
                    }
                }
                else if (ret == EXAMPLE_ESPNOW_DATA_UNICAST) {
                    ESP_LOGI(TAG, "Receive %dth unicast data from: "MACSTR", len: %d", recv_seq, MAC2STR(recv_cb->mac_addr), recv_cb->data_len);//在这里打印收到的消息
                    ESP_LOGI(TAG, "Data is: %d", recv_payload);//打印的东西
                    /* If receive unicast ESPNOW data, also stop sending broadcast ESPNOW data. */
                    send_param->broadcast = false;//控制是否发送broadcast寻找节点
                }
                else {
                    ESP_LOGI(TAG, "Receive error data from: "MACSTR"", MAC2STR(recv_cb->mac_addr));
                }
                break;
            }
            default:
                ESP_LOGE(TAG, "Callback type error: %d", evt.id);
                break;
        }
    }
}
/*First init*/
static esp_err_t example_espnow_init(void)
{
    example_espnow_send_param_t *send_param;

    s_example_espnow_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(example_espnow_event_t));
    if (s_example_espnow_queue == NULL) {
        ESP_LOGE(TAG, "Create mutex fail");
        return ESP_FAIL;
    }

    /* Initialize ESPNOW and register sending and receiving callback function. */
    ESP_ERROR_CHECK( esp_now_init() );
    ESP_ERROR_CHECK( esp_now_register_send_cb(example_espnow_send_cb) );
    ESP_ERROR_CHECK( esp_now_register_recv_cb(example_espnow_recv_cb) );

    /* Set primary master key. */
    ESP_ERROR_CHECK( esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK) );

    /* Add 1 broadcast peer information to peer list. */
    esp_now_peer_info_t *peer = malloc(sizeof(esp_now_peer_info_t));
    if (peer == NULL) {
        ESP_LOGE(TAG, "Malloc peer information fail");
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    memset(peer, 0, sizeof(esp_now_peer_info_t));
    peer->channel = CONFIG_ESPNOW_CHANNEL;
    peer->ifidx = ESPNOW_WIFI_IF;
    peer->encrypt = false;
    /*add your mac here*/
    //memcpy(peer->peer_addr, s_example_broadcast_mac, ESP_NOW_ETH_ALEN);
    memcpy(peer->peer_addr, esp32_mac2, ESP_NOW_ETH_ALEN);
    //memcpy(peer->peer_addr, esp32_mac2, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK( esp_now_add_peer(peer) );
    free(peer);

    /* Initialize sending parameters. */
    send_param = malloc(sizeof(example_espnow_send_param_t));
    memset(send_param, 0, sizeof(example_espnow_send_param_t));
    if (send_param == NULL) {
        ESP_LOGE(TAG, "Malloc send parameter fail");
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    //send_param->unicast = false;
    send_param->unicast = true;
    //send_param->broadcast = true;
    send_param->broadcast = false;
    //send_param->state = 0;
    send_param->state = 1;
    //send_param->magic = esp_random();
    //send_param->magic = 7777;//小
    send_param->magic = 77777;//大
    send_param->count = CONFIG_ESPNOW_SEND_COUNT;
    send_param->delay = CONFIG_ESPNOW_SEND_DELAY;
    send_param->len = CONFIG_ESPNOW_SEND_LEN;
    send_param->buffer = malloc(CONFIG_ESPNOW_SEND_LEN);//Set the data you wanna send
    if (send_param->buffer == NULL) {
        ESP_LOGE(TAG, "Malloc send buffer fail");
        free(send_param);
        vSemaphoreDelete(s_example_espnow_queue);
        esp_now_deinit();
        return ESP_FAIL;
    }
    //memcpy(send_param->dest_mac, s_example_broadcast_mac, ESP_NOW_ETH_ALEN);//在这里把地址拷贝到dest_mac里了
    memcpy(send_param->dest_mac,esp32_mac2, ESP_NOW_ETH_ALEN);//在这里把地址拷贝到dest_mac里了
    example_espnow_data_prepare(send_param);//准备发送的数据

    xTaskCreate(example_espnow_task, "example_espnow_task", 2048, send_param, 4, NULL);

    return ESP_OK;
}

static void example_espnow_deinit(example_espnow_send_param_t *send_param)
{
    free(send_param->buffer);
    free(send_param);
    vSemaphoreDelete(s_example_espnow_queue);
    esp_now_deinit();
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    example_wifi_init();
    example_espnow_init();
}
