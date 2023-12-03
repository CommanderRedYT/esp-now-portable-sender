constexpr const char * const TAG = "espnow";
#include "espnow.h"
#include "sdkconfig.h"

// system includes
#include <cctype>
#include <string>

// esp-idf includes
#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_now.h>
#include <nvs_flash.h>
#include <esp_wifi.h>

// local includes
#include "sleeper.h"

namespace espnow {

bool messageSent{false};

void str2mac(const char * str, uint8_t * mac)
{
    int i = 0;
    while (*str)
    {
        if (isxdigit(*str))
        {
            mac[i++] = strtoul(str, nullptr, 16);
            str += 2;
        }
        else
        {
            ++str;
        }
    }
}

void init()
{
    ESP_LOGI(TAG, "init");

    // initialize nvs
    if (const auto res = nvs_flash_init(); res != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_flash_init: %s", esp_err_to_name(res));
        return;
    }

    // initialize wi-fi
    if (const auto res = esp_netif_init(); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_netif_init: %s", esp_err_to_name(res));
        return;
    }

    if (const auto res = esp_event_loop_create_default(); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_event_loop_create_default: %s", esp_err_to_name(res));
        return;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (const auto res = esp_wifi_init(&cfg); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_wifi_init: %s", esp_err_to_name(res));
        return;
    }

    if (const auto res = esp_wifi_set_storage(WIFI_STORAGE_RAM); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_wifi_set_storage: %s", esp_err_to_name(res));
        return;
    }

    if (const auto res = esp_wifi_set_mode(WIFI_MODE_AP); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_wifi_set_mode: %s", esp_err_to_name(res));
        return;
    }

    if (const auto res = esp_wifi_start(); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_wifi_start: %s", esp_err_to_name(res));
        return;
    }

    if (const auto res = esp_wifi_set_channel(CONFIG_PORTABLE_SENDER_CHANNEL, WIFI_SECOND_CHAN_NONE); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_wifi_set_channel: %s", esp_err_to_name(res));
        return;
    }

    if (const auto res = esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_wifi_set_protocol: %s", esp_err_to_name(res));
        return;
    }

    // initialize esp-now
    if (const auto res = esp_now_init(); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_now_init: %s", esp_err_to_name(res));
        return;
    }

    auto* peer = new esp_now_peer_info_t;
    peer->channel = CONFIG_PORTABLE_SENDER_CHANNEL;
    peer->ifidx = WIFI_IF_AP;
    peer->encrypt = false;
    str2mac(CONFIG_PORTABLE_SENDER_RECEIVER_MAC, peer->peer_addr);

    if (const auto res = esp_now_add_peer(peer); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_now_add_peer: %s", esp_err_to_name(res));
        return;
    }
    else
    {
        ESP_LOGI(TAG, "esp_now_add_peer: %s, mac: %02x:%02x:%02x:%02x:%02x:%02x", esp_err_to_name(res),
            peer->peer_addr[0], peer->peer_addr[1], peer->peer_addr[2], peer->peer_addr[3], peer->peer_addr[4], peer->peer_addr[5]);
    }

    delete peer;

    if (const auto res = esp_now_register_send_cb([](const uint8_t * mac, esp_now_send_status_t status) {
        if (status == ESP_NOW_SEND_FAIL)
        {
            messageSent = false;
        }
        else
        {
            sleeper::go_to_sleep();
        }

        ESP_LOGI(TAG, "esp_now_register_send_cb: %s", esp_err_to_name(status));
    }); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_now_register_send_cb: %s", esp_err_to_name(res));
        return;
    }
}

void update()
{
    if (messageSent)
    {
        return;
    }

    auto message = std::string{CONFIG_PORTABLE_SENDER_MESSAGE};
    uint8_t mac[ESP_NOW_ETH_ALEN];
    str2mac(CONFIG_PORTABLE_SENDER_RECEIVER_MAC, mac);

    if (const auto res = esp_now_send(mac, reinterpret_cast<const uint8_t *>(message.data()), message.size()); res != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_now_send: %s", esp_err_to_name(res));
        return;
    }

    messageSent = true;
}

} // namespace espnow
