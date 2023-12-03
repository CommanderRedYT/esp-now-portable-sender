constexpr const char * const TAG = "sleeper";
#include "sleeper.h"

// esp-idf includes
#include <esp_log.h>
#include <esp_sleep.h>

namespace sleeper {

bool can_sleep{false};

void go_to_sleep()
{
    can_sleep = true;
}

void init()
{
    ESP_LOGI(TAG, "init");
    esp_sleep_enable_ext0_wakeup(gpio_num_t(CONFIG_PORTABLE_SENDER_WAKEUP_PIN), ESP_EXT1_WAKEUP_ANY_HIGH);
}

void update()
{
    if (can_sleep) {
        can_sleep = false;
        ESP_LOGI(TAG, "going to bed, good night!");
        esp_deep_sleep_start();
    }
}

} // namespace sleeper
