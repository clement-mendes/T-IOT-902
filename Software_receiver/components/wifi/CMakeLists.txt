idf_component_register(SRCS "src/wifi.c"
                    PRIV_REQUIRES esp_wifi esp_event nvs_flash
                    INCLUDE_DIRS "include")