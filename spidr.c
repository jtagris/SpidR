#include <stdio.h>
#include <stdlib.h>
#include <rtl-sdr.h>

#define DEFAULT_SAMPLE_RATE 2.4e6  // Taux d'échantillonnage en Hz
#define DEFAULT_FREQUENCY_MIN 88e6 // Fréquence minimale en Hz
#define DEFAULT_FREQUENCY_MAX 108e6 // Fréquence maximale en Hz
#define DEFAULT_THRESHOLD 25000    // Seuil de détection du signal
#define DEFAULT_GAIN 49            // Gain RF en dB (entier)

int main() {
    int i, ret;
    int device_count;
    uint32_t frequency;
    uint32_t sample_rate = DEFAULT_SAMPLE_RATE;
    uint32_t frequency_min = DEFAULT_FREQUENCY_MIN;
    uint32_t frequency_max = DEFAULT_FREQUENCY_MAX;
    int16_t *samples = NULL;
    uint32_t length = 0;
    uint32_t threshold = DEFAULT_THRESHOLD;
    int gain = DEFAULT_GAIN;
    uint32_t step_size = 1e6; // Pas de balayage en Hz

    // Initialize device
    ret = rtlsdr_get_device_count();
    if (ret < 0) {
        fprintf(stderr, "Aucun périphérique RTL-SDR trouvé\n");
        return 1;
    }

    // Open device
    rtlsdr_dev_t *dev;
    ret = rtlsdr_open(&dev, 0); // Utiliser le premier périphérique
    if (ret < 0) {
        fprintf(stderr, "Impossible d'ouvrir le périphérique RTL-SDR\n");
        return 1;
    }

    // Set sample rate
    ret = rtlsdr_set_sample_rate(dev, sample_rate);
    if (ret < 0) {
        fprintf(stderr, "Impossible de définir le taux d'échantillonnage\n");
        rtlsdr_close(dev);
        return 1;
    }

    // Allouer de la mémoire pour les échantillons
    length = (uint32_t)(sample_rate * 2); // Deux secondes d'échantillons
    samples = (int16_t *)malloc(length * sizeof(int16_t));
    if (!samples) {
        fprintf(stderr, "Impossible d'allouer de la mémoire\n");
        rtlsdr_close(dev);
        return 1;
    }

    printf("Balayage des fréquences de %.2f MHz à %.2f MHz...\n", frequency_min / 1e6, frequency_max / 1e6);

    for (frequency = frequency_min; frequency <= frequency_max; frequency += step_size) {
        // Set frequency
        ret = rtlsdr_set_center_freq(dev, frequency);
        if (ret < 0) {
            fprintf(stderr, "Impossible de définir la fréquence centrale\n");
            free(samples);
            rtlsdr_close(dev);
            return 1;
        }

        // Lire les échantillons
        ret = rtlsdr_read_sync(dev, samples, length, NULL);
        if (ret < 0) {
            fprintf(stderr, "Impossible de lire les échantillons\n");
            free(samples);
            rtlsdr_close(dev);
            return 1;
        }

        // Traitement des échantillons
        for (i = 0; i < length; i++) {
            // Vérifier si le signal dépasse le seuil
            if (abs(samples[i]) > threshold) {
                printf("Signal détecté à %.2f MHz\n", (float)frequency / 1e6);
                break;
            }
        }
    }

    // Libérer la mémoire et fermer le périphérique
    free(samples);
    rtlsdr_close(dev);

    return 0;
}
