#include "unity.h"
#include "hello.h"

void test_print_hello() {
    TEST_ASSERT_EQUAL(0, 0); // Exemple de test vide
}

void app_main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_print_hello);
    UNITY_END();
}