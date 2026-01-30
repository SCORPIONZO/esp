idf_component_register(
    SRCS "web_server_main.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_http_server nvs_flash esp_event esp_netif
    PRIV_REQUIRES esp_timer
)