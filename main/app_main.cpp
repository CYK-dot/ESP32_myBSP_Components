#include <stdio.h>
#include <string.h>
#include <esp_log.h>

#include "easy_nvs.h"
#include "ewifi_basic.h"
#include "eespnow.h"
#include "elink_now.h"

static const char *TAG = "main";

void cb(NowRemoteMessage_t msg,NowRemoteAddr_t addr,NowRemoteCtrl_t ctrl)
{
    ESP_LOGW(TAG,"ESP-NOW收到:%s",(char*)msg.payload);
}

extern "C" void app_main()
{
    elink_now_init_example_host();
}
