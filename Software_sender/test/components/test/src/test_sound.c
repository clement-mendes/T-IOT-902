#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "sound.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "test_sound.h"

static int tests_passed = 0;

void setUp_sound(void)
{
    sound_init();
}


void test_sound_init(void)
{
    sound_init();
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
    TEST_PASS();
}

void test_sound_measure_db(void)
{
    float db = sound_measure_db();
    TEST_ASSERT_GREATER_OR_EQUAL(30.0f, db);
    TEST_ASSERT_LESS_OR_EQUAL(90.0f, db);
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
}

void test_sound_get_average_db(void)
{
    float avg_db = sound_get_average_db();
    TEST_ASSERT_GREATER_OR_EQUAL(30.0f, avg_db);
    TEST_ASSERT_LESS_OR_EQUAL(90.0f, avg_db);
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
} 