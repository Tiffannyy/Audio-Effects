#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <unistd.h>
#include "peripherals.h"
#include "Engine_test.h"

int main(int argc, char* argv[]) {

    /* Initialize peripherals via wiringPi */
    if (initializePeripherals() < 0) {
        printf("Failed to setup peripherals.\n");
        return 1;
    }
    
    while (1) {
        readPeripherals();
        printf("V: %.2f | M: %.2f | ", PERIPHERAL_DATA.volumeValue, PERIPHERAL_DATA.mixValue);
        
        if (MENU_MODE == EFFECT_SELECTION_MODE) {
            printf("\e[0;93mSelecting effect\e[0m | ");
            switch (EFFECT_SELECTION) {
                case NO_EFFECT:  printf("No effect"); break;
                case TREMOLO:    printf("Tremolo"); break;
                case DELAY:      printf("Delay"); break;
                case REVERB:     printf("Reverb"); break;
                case BITCRUSH:   printf("Bitcrush"); break;
                case OVERDRIVE:  printf("Overdrive"); break;
                case DISTORTION: printf("Distortion"); break;
                case FUZZ:       printf("Fuzz"); break;
            }
        }
        
        else if (MENU_MODE == AUDIO_PARAM_SELECTION_MODE) {
            printf("\e[0;96mSelecting parameter\e[0m | ");
            switch (EFFECT_SELECTION) {
                // too lazy to print proper names of enums rn
                case TREMOLO:    printf("%d", AUDIO_PARAM_SELECTION.TREMOLO); break;
                case DELAY:      printf("%d", AUDIO_PARAM_SELECTION.DELAY); break;
                case REVERB:     printf("%d", AUDIO_PARAM_SELECTION.REVERB); break;
                case BITCRUSH:   printf("%d", AUDIO_PARAM_SELECTION.BITCRUSH); break;
                case OVERDRIVE:  printf("%d", AUDIO_PARAM_SELECTION.OVERDRIVE); break;
                case DISTORTION: printf("%d", AUDIO_PARAM_SELECTION.DISTORTION); break;
                case FUZZ:       printf("%d", AUDIO_PARAM_SELECTION.FUZZ); break;
            }
        }
        
        else if (MENU_MODE == AUDIO_PARAM_VALUE_MODE) {
            printf("\e[0;92mAdjusting parameter\e[0m | ");
        }
        
        printf("              \r");
    }

}
