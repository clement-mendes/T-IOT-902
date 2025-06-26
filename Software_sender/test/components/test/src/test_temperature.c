#include <stdio.h>
#include <string.h>
#include "unity.h"
#include "temperature.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "test_temperature.h"

static int tests_passed = 0;

void setUp_temperature(void)
{
    temperature_init();
}

void test_temperature_init(void)
{
    temperature_init();
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
    TEST_PASS();
}

void test_temperature_get(void)
{
    float temp = temperature_get();
    TEST_ASSERT_GREATER_OR_EQUAL(-40.0f, temp);
    TEST_ASSERT_LESS_OR_EQUAL(85.0f, temp);
    TEST_ASSERT_NOT_EQUAL(0.0f, temp);
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
}

void test_pressure_get(void)
{
    float pressure = pressure_get();
    TEST_ASSERT_GREATER_OR_EQUAL(300.0f, pressure);
    TEST_ASSERT_LESS_OR_EQUAL(1200.0f, pressure);
    TEST_ASSERT_NOT_EQUAL(0.0f, pressure);
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
}

void test_multiple_readings(void)
{
    float temp1 = temperature_get();
    float temp2 = temperature_get();
    float temp3 = temperature_get();
    TEST_ASSERT_FLOAT_WITHIN(5.0f, temp1, temp2);
    TEST_ASSERT_FLOAT_WITHIN(5.0f, temp2, temp3);
    float press1 = pressure_get();
    float press2 = pressure_get();
    float press3 = pressure_get();
    TEST_ASSERT_FLOAT_WITHIN(10.0f, press1, press2);
    TEST_ASSERT_FLOAT_WITHIN(10.0f, press2, press3);
    printf("Test %s PASSED\n", __func__);
    tests_passed++;
} 