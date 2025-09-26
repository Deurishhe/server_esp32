#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

#define WIFI_SSID      "SAMPLE"
#define WIFI_PASS      "SAMPLE"

#define WIFI_MAXIMUM_RETRY 5
#define HTTP_PORT 80
static const char *TAG = "WIFI_HTTP_SERVER";

static int s_retry_num = 0;

const static char PONG_RESPONSE[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 4\r\n\r\npong";
const static char NOT_FOUND_RESPONSE[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";

static void http_server_task(void *pvParameters) {
    char recv_buf[1024];
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        ESP_LOGE(TAG, "Не удалось создать сокет HTTP сервера");
        vTaskDelete(NULL);
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(HTTP_PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        ESP_LOGE(TAG, "Не удалось привязать сокет HTTP сервера");
        close(server_socket);
        vTaskDelete(NULL);
        return;
    }

    if (listen(server_socket, 5) != 0) {
        ESP_LOGE(TAG, "Ошибка прослушивания сокета HTTP сервера");
        close(server_socket);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "HTTP сервер запущен и слушает порт %d", HTTP_PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            ESP_LOGE(TAG, "Не удалось принять соединение");
            continue;
        }
        ESP_LOGI(TAG, "HTTP: Соединение принято");

        int len = recv(client_socket, recv_buf, sizeof(recv_buf) - 1, 0);
        
        if (len > 0) {
            recv_buf[len] = '\0';
            
            if (strstr(recv_buf, "GET /ping")) {
                ESP_LOGI(TAG, "HTTP: Получен запрос /ping. Отправляем pong.");
                send(client_socket, PONG_RESPONSE, sizeof(PONG_RESPONSE) - 1, 0);
            } else {
                ESP_LOGI(TAG, "HTTP: Получен неизвестный запрос. Отправляем 404.");
                send(client_socket, NOT_FOUND_RESPONSE, sizeof(NOT_FOUND_RESPONSE) - 1, 0);
            }
        }
        
        close(client_socket);
    }
    close(server_socket);
    vTaskDelete(NULL);
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    ESP_LOGE(TAG, "Отключение от Wi-Fi.");

    if (s_retry_num < WIFI_MAXIMUM_RETRY) {
        ++s_retry_num;
        ESP_LOGI(TAG, "Повторная попытка подключения к Wi-Fi (%d/%d)...", s_retry_num, WIFI_MAXIMUM_RETRY);
        esp_wifi_connect();
    } else {
        ESP_LOGE(TAG, "Превышено количество попыток подключения.");
    }
}

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI(TAG, "УСПЕШНО ПОДКЛЮЧЕНО К WI-FI!");
    ESP_LOGI(TAG, "ПОЛУЧЕН IP-АДРЕС: " IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
}

void wifi_init_sta(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
    
    ESP_LOGI(TAG, "Процесс инициализации Wi-Fi запущен, первая попытка подключения...");
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    wifi_init_sta();
    xTaskCreate(http_server_task, "HTTP Server", 4096, NULL, 5, NULL);
}
