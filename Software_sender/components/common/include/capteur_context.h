   #ifndef CAPTEUR_CONTEXT_H
   #define CAPTEUR_CONTEXT_H

   #include "freertos/FreeRTOS.h"
   #include "freertos/semphr.h"

   typedef struct {
       float *buffer;
       int sample_count;
       float average;
       SemaphoreHandle_t done_semaphore;
       SemaphoreHandle_t start_signal;
   } CapteurContext;

   #endif