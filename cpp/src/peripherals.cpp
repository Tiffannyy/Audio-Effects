/*
 * peripherals.cpp
 * 
 * Nathaniel Kalaw
 * 9 February 2026
 * 
 * Description: Functions for handling GPIO peripherals
 * via wiringPi.
 */

#include <cstdint>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "../include/peripherals.h"

#define ADC_ADDRESS 0x4b
#define DEBOUNCE_MS 200
#define CLK_PIN     28
#define DT_PIN      27
#define SW_PIN      29
#define VOLUME_KNOB 0
#define MIX_KNOB    1


// ============================================================
// [VARIABLES]


// Potentiometer ADC variables
int adcFD;
const static uint8_t adcCommands[8] = {
    0x84, 0xC4, 0x94, 0xD4,
    0xA4, 0xE4, 0xB4, 0xF4
};

// Encoder variables
int flag;
double currPressTime, prevPressTime;

// Peripheral variables
pthread_t potentiometerThread, encoderThread;
PeripheralData PERIPHERAL_DATA;


// ============================================================
// [HELPER FUNCTIONS]


double getCurrTimestamp() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    double timestamp = (double) ts.tv_sec + (double) ts.tv_nsec / 1000000000.0;
    return timestamp;
}


int readPotentiometer(int adc_channel) {
    wiringPiI2CWrite(adcFD, adcCommands[adc_channel]);
    return (int) wiringPiI2CRead(adcFD);
}



// ============================================================
// [FUNCTIONS]


int initializePeripherals(void) {

    /* Setup potentionmeter ADC */
    if ((adcFD = wiringPiI2CSetup(ADC_ADDRESS)) < 0)
        return -1;

    /* Setup rotary encoder */
    if (wiringPiSetup() < 0)
        return -2;

    /* Initialize encoder data and button GPIO pins */
    pinMode(CLK_PIN, INPUT);
    pinMode(DT_PIN, INPUT);
    pinMode(SW_PIN, INPUT);
    pullUpDnControl(SW_PIN, PUD_UP);

    /* Initialize ISR for button */
    if (wiringPiISR(SW_PIN, INT_EDGE_FALLING, &runEncoderButtonISR) < 0)
        return -3;
    prevPressTime = getCurrTimestamp();
        
    /* Initiaize and run threads for peripherals */
    int rc = pthread_create(&potentiometerThread, NULL, runPotentiometerThread, NULL);
    if (rc) {
        printf("Failed to create potentiometer thread. (%d\n)", rc);
        return -4;
    }    
    rc = pthread_create(&encoderThread, NULL, runEncoderThread, NULL);
    if (rc) {
        printf("Failed to create encoder thread. (%d\n)", rc);
        return -5;
    }

    return 0;    
}


void* runPotentiometerThread(void* args) {
    while (1) { 
        PERIPHERAL_DATA.volumeValue = (255 - readPotentiometer(VOLUME_KNOB)) / 255.0;
        PERIPHERAL_DATA.mixValue = (255 - readPotentiometer(MIX_KNOB)) / 255.0;
    }
}


void* runEncoderThread(void* args) {
    while (1) {

        int dtLast = digitalRead(DT_PIN);
        int dtCurr;

        while (!digitalRead(CLK_PIN)) {
            dtCurr = digitalRead(DT_PIN);
            flag = 1;
        }

        if (flag == 1) {
            flag = 0;
            if (dtLast == 0 && dtCurr == 1) // Turned left
                PERIPHERAL_DATA.encoderTurned = -1;
            if (dtLast == 1 && dtCurr == 0) // Turned right
                PERIPHERAL_DATA.encoderTurned = 1;
        }

    }
}


void runEncoderButtonISR(void) {
    currPressTime = getCurrTimestamp();
    if ((currPressTime - prevPressTime)*1000 > DEBOUNCE_MS) {
        PERIPHERAL_DATA.encoderPressed = true;
        prevPressTime = currPressTime;
    }
}


// TODO: Make safe close peripheral function
// int close_peripherals(void) {};







