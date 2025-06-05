idf_component_register(
    SRCS "test_sound.c" "test_temperature.c"
    INCLUDE_DIRS "."
    REQUIRES "sound" "temperature" "unity"
)
