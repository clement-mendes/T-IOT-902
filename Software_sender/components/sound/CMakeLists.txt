idf_component_register(SRCS "src/sound.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_i2s driver common)