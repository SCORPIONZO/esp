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

static const char *TAG = "WEBSERVER";

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

/* An HTTP GET handler */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    const char *resp_str = 
        "<!DOCTYPE html>"
        "<html>"
        "<head><title>ESP32 Web Server</title></head>"
        "<body>"
        "<h1>ESP32 Web Server</h1>"
        "<p>Welcome to the ESP32 Web Server!</p>"
        "<p><a href=\"/status\">Check System Status</a></p>"
        "</body>"
        "</html>";

    httpd_resp_sendstr(req, resp_str);
    return ESP_OK;
}

static esp_err_t status_get_handler(httpd_req_t *req)
{
    char response[512];
    
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_free_heap = esp_get_minimum_free_heap_size();
    uint32_t uptime = esp_timer_get_time() / 1000000;
    
    snprintf(response, sizeof(response),
             "{"
             "\"uptime\": %lu,"
             "\"free_heap\": %lu,"
             "\"min_free_heap\": %lu"
             "}", 
             uptime, 
             free_heap, 
             min_free_heap);

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

        return server;
    }

    ESP_LOGI(TAG, "Error starting HTTP server!");
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
    ESP_LOGI(TAG, "Starting ESP32 Web Server");

    initialise_wifi();
    start_webserver();
}