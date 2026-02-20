/* 
 * menu.h
 * DSP program
 *
 * Tiffany Liu
 * 4 June 2025
 * 
 * Description: contains the menu function for main.cpp
 * 
*/

#pragma once

#include "types.h"

// Choice function
void choiceSelect(char choice, EffectChoices &effectChoice, bool &validChoice, bool &exitFlag);

// Menu function
bool menuFunction(EffectChoices &effectChoice);
