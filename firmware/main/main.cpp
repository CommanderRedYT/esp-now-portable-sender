constexpr const char * const TAG = "main";

#include <esp_log.h>

extern "C" [[noreturn]] void app_main()
{
    ESP_LOGI(TAG, "Hello world");

    while (true) {}
}
