/*
 * peripherals.h
 * 
 * Nathaniel Kalaw
 * 24 February 2026
 * 
 * Description: Functions for handling GPIO peripherals
 * via wiringPi.
 */

#pragma once


// ============================================================
// [VARIABLES]


struct PeripheralData {
	float VOLUME_VALUE = 0.0f;
	float MIX_VALUE = 0.0f;
	short ENCODER_TURNED  = 0; // -1 = turned left, 0 = idle, 1 = turned right
	bool  ENCODER_PRESSED = false;
};

extern PeripheralData peripheralData;


// ============================================================
// [FUNCTIONS]


int initializePeripherals();

void* runPotentiometerThread(void* args);

void* runEncoderThread(void* args);

void runEncoderButtonISR();

void closePeripherals();


