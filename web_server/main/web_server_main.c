#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_timer.h"  // Добавляем недостающий заголовочный файл
#include "esp_chip_info.h"  // Добавляем заголовочный файл для esp_chip_info_t
#include "driver/gpio.h"  // Добавляем библиотеку GPIO
#include "sdkconfig.h"

static const char *TAG = "WEBSERVER";

/* Определение GPIO пина для встроенного светодиода, с fallback значением */
#ifndef CONFIG_BLINK_GPIO
#define BLINK_GPIO 2
#else
#define BLINK_GPIO CONFIG_BLINK_GPIO
#endif

/* WiFi configuration */
static esp_netif_t *netif_instance = NULL;
wifi_config_t wifi_config = {
    .ap = {
        .ssid = "ESP32-WebServer",
        .ssid_len = 0,
        .channel = 1,
        .password = "123456789",
        .max_connection = 4,
        .authmode = WIFI_AUTH_WPA2_PSK,
    },
};

/* Переменная для хранения состояния светодиода */
static bool led_state = false;

/* Функция инициализации GPIO для светодиода */
static void blink_gpio_init(void)
{
    gpio_reset_pin(BLINK_GPIO);
    /* Устанавливаем GPIO пин как выход */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    /* Устанавливаем начальное состояние в LOW (выключен) */
    gpio_set_level(BLINK_GPIO, 0);
}

/* Функция установки состояния светодиода */
static void set_led_state(bool state)
{
    gpio_set_level(BLINK_GPIO, state ? 1 : 0);
    led_state = state;
    ESP_LOGI(TAG, "LED state set to %s", state ? "ON" : "OFF");
}

/* An HTTP GET handler */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    // Получаем текущее состояние светодиода для отображения
    const char *led_button_state = led_state ? "Выключить LED" : "Включить LED";
    const char *led_display_state = led_state ? "включен" : "выключен";
    
    char response[1024];
    snprintf(response, sizeof(response),
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<title>ESP32 Web Server</title>"
        "<style>"
        "body { font-family: Arial, sans-serif; margin: 40px; }"
        "button { cursor: pointer; }"
        ".led-control { margin: 10px 0; }"
        "</style>"
        "</head>"
        "<body>"
        "<h1>ESP32 Web Server</h1>"
        "<p>Welcome to the ESP32 Web Server!</p>"
        "<div class=\"led-control\">"
        "<h2>Управление светодиодом</h2>"
        "<p>Состояние светодиода: <strong>%s</strong></p>"
        "<a href=\"/ledon\"><button style=\"font-size:18px;padding:10px;margin:5px;width:150px;\">%s</button></a>"
        "<a href=\"/ledoff\"><button style=\"font-size:18px;padding:10px;margin:5px;width:150px;\">Выключить LED</button></a>"
        "<a href=\"/ledtoggle\"><button style=\"font-size:18px;padding:10px;margin:5px;width:150px;\">Переключить LED</button></a>"
        "<br><br>"
        "<a href=\"/status\"><button style=\"font-size:18px;padding:10px;margin:5px;width:200px;\">Статус системы</button></a>"
        "</div>"
        "</body>"
        "</html>", 
        led_display_state, 
        led_button_state);

    httpd_resp_sendstr(req, response);
    return ESP_OK;
}

/* Обработчик для включения светодиода */
static esp_err_t ledon_get_handler(httpd_req_t *req)
{
    set_led_state(true);
    
    const char *resp_str = 
        "<!DOCTYPE html>"
        "<html>"
        "<head><meta charset=\"UTF-8\"><title>LED Control</title></head>"
        "<body>"
        "<h1>Светодиод включен!</h1>"
        "<p><a href=\"/\">Назад к главной странице</a></p>"
        "</body>"
        "</html>";

    httpd_resp_sendstr(req, resp_str);
    return ESP_OK;
}

/* Обработчик для выключения светодиода */
static esp_err_t ledoff_get_handler(httpd_req_t *req)
{
    set_led_state(false);
    
    const char *resp_str = 
        "<!DOCTYPE html>"
        "<html>"
        "<head><meta charset=\"UTF-8\"><title>LED Control</title></head>"
        "<body>"
        "<h1>Светодиод выключен!</h1>"
        "<p><a href=\"/\">Назад к главной странице</a></p>"
        "</body>"
        "</html>";

    httpd_resp_sendstr(req, resp_str);
    return ESP_OK;
}

/* Обработчик для переключения светодиода */
static esp_err_t ledtoggle_get_handler(httpd_req_t *req)
{
    set_led_state(!led_state);
    
    const char *resp_str = 
        "<!DOCTYPE html>"
        "<html>"
        "<head><meta charset=\"UTF-8\"><title>LED Control</title></head>"
        "<body>"
        "<h1>Светодиод переключен!</h1>"
        "<p>Текущее состояние: <strong>%s</strong></p>"
        "<p><a href=\"/\">Назад к главной странице</a></p>"
        "</body>"
        "</html>";
    
    char response[512];
    snprintf(response, sizeof(response), resp_str, led_state ? "включен" : "выключен");

    httpd_resp_sendstr(req, response);
    return ESP_OK;
}

static esp_err_t status_get_handler(httpd_req_t *req)
{
    char response[512];
    
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_free_heap = esp_get_minimum_free_heap_size();
    uint32_t uptime = esp_timer_get_time() / 1000000;
    
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    snprintf(response, sizeof(response),
             "{"
             "\"uptime\": %lu,"
             "\"free_heap\": %lu,"
             "\"min_free_heap\": %lu,"
             "\"led_state\": %s,"
             "\"chip_model\": \"%s\","
             "\"cores\": %d,"
             "\"features\": \"%s\""
             "}", 
             uptime, 
             free_heap, 
             min_free_heap,
             led_state ? "true" : "false",
             chip_info.model == CHIP_ESP32 ? "ESP32" : "Other",
             chip_info.cores,
             chip_info.features & CHIP_FEATURE_WIFI_BGN ? "WiFi" : "No WiFi");

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Registering URI handlers");

        httpd_uri_t root = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root);

        httpd_uri_t status = {
            .uri       = "/status",
            .method    = HTTP_GET,
            .handler   = status_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &status);

        // Регистрируем новые обработчики для управления светодиодом
        httpd_uri_t ledon = {
            .uri       = "/ledon",
            .method    = HTTP_GET,
            .handler   = ledon_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &ledon);

        httpd_uri_t ledoff = {
            .uri       = "/ledoff",
            .method    = HTTP_GET,
            .handler   = ledoff_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &ledoff);
        
        httpd_uri_t ledtoggle = {
            .uri       = "/ledtoggle",
            .method    = HTTP_GET,
            .handler   = ledtoggle_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &ledtoggle);

        return server;
    }

    ESP_LOGE(TAG, "Error starting HTTP server!");
    return NULL;
}

static void initialise_wifi(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Initialize WiFi */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Create default WiFi AP */
    netif_instance = esp_netif_create_default_wifi_ap();
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started with SSID: %s", wifi_config.ap.ssid);
    
    /* Get and print IP address */
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(netif_instance, &ip_info);
    ESP_LOGI(TAG, "Connect to WiFi SSID '%s' with password '%s'", 
             wifi_config.ap.ssid, wifi_config.ap.password);
    ESP_LOGI(TAG, "Access the web server at http://" IPSTR, IP2STR(&ip_info.ip));
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting ESP32 Web Server with LED control");

    // Инициализируем GPIO для светодиода
    blink_gpio_init();
    // Устанавливаем начальное состояние светодиода (выключен)
    set_led_state(false);

    initialise_wifi();
    
    httpd_handle_t server = start_webserver();
    if (server == NULL) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return;
    }
    
    ESP_LOGI(TAG, "Web server with LED control started successfully");
}