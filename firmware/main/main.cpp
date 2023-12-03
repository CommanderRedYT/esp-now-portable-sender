constexpr const char * const TAG = "main";

// esp-idf includes
#include <esp_log.h>
#include <freertos/FreeRTOS.h>

// local includes
#include "espnow.h"
#include "sleeper.h"

extern "C" [[noreturn]] void app_main()
{
    ESP_LOGI(TAG, "Hello world");

    sleeper::init();
    espnow::init();

    while (true) {
        espnow::update();
        sleeper::update();

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
