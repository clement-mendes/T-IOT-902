#include <stdio.h>
#include "log_utils.h"

void log_info(const char* message) {
    printf("[INFO] %s\n", message);
}

void log_error(const char* message) {
    printf("[ERROR] %s\n", message);
} 