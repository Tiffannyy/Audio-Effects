/*
 * peripherals.h
 * 
 * Nathaniel Kalaw
 * 9 February 2026
 * 
 * Description: Functions for handling GPIO peripherals
 * via wiringPi.
 */

#pragma once


// ============================================================
// [VARIABLES]


struct PeripheralData {
	float volumeValue = 0.0f;
	float mixValue = 0.0f;
	short encoderTurned  = 0; // -1 = turned left, 0 = idle, 1 = turned right
	bool  encoderPressed = false;
};

extern PeripheralData PERIPHERAL_DATA;


// ============================================================
// [FUNCTIONS]


int initializePeripherals(void);

void* runPotentiometerThread(void* args);

void* runEncoderThread(void* args);

void runEncoderButtonISR(void);

int closePeripherals(void);






