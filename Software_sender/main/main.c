#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sound.h"

#define BUFFER_SIZE 1024

void app_main(void)
{
    // Initialisation du module son
    sound_init();
    
    // Allocation d'un buffer pour les échantillons audio
    int16_t audio_buffer[BUFFER_SIZE];

    while (1)
    {
        // Lecture des données audio
        int samples_read = sound_read(audio_buffer, BUFFER_SIZE);
        
        // Vérification si le capteur n'est pas connecté ou s'il y a une erreur
        if (samples_read <= 0) {
            // Affichage d'un message d'erreur si la lecture échoue
            printf("Capteur non connecté ou erreur de lecture\n");
            vTaskDelay(pdMS_TO_TICKS(500));  // Délai pour éviter un message d'erreur trop fréquent
            continue;
        }

        // Calcul du RMS (Root Mean Square)
        long sum = 0;
        for (int i = 0; i < samples_read; i++) {
            sum += audio_buffer[i] * audio_buffer[i];
        }

        // S'assurer que la somme est positive avant de calculer le RMS
        if (sum > 0) {
            float rms = sqrt((float)sum / samples_read);

            // Conversion en dB
            // La valeur de référence peut être calibrée en fonction de votre configuration.
            // Ici, on considère une référence arbitraire (par exemple 1.0).
            if (rms > 0) {
                float db = 20 * log10(rms);
                printf("Niveau sonore: %.2f dB\n", db);
            } else {
                printf("Niveau sonore: trop faible pour être mesuré\n");
            }
        } else {
            printf("Niveau sonore: trop faible pour être mesuré\n");
        }

        // Délai pour limiter la fréquence d'affichage (ex. 500 ms)
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
