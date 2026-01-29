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

#include <iostream>

#include "types.h"

// Choice function
void choiceSelect(std::string choice, EffectChoices &effectChoice, bool &validChoice){
    switch (choice){
        case '0':
            std::cout << "Ending program..." << std::endl;
            return false; // request graceful exit to caller
            break;
        case '1':                       // no effect
            effectChoice.norm = true;
            validChoice = true;
            break;
        case '2':                       // tremolo
            effectChoice.trem = true;
            validChoice = true;
            break;
        case '3':
            effectChoice.delay = true;  // delay
            validChoice = true;
            break;
        case '4':
            effectChoice.reverb = true; // reverb
            validChoice = true;
            break;
        case '5':
            effectChoice.bitcrush = true; // bitcrush
            validChoice = true;
            break;
        case '6':
            effectChoice.dirt = true;
            validChoice = true;
        default:
            break;
    }
}

// Menu function
bool menuFunction(EffectChoices &effectChoice){
    bool validChoice = false;
    char userChoice;
    std::cout << "*------ Audio Effects Program ------*\n" << "Effects Options:\n";
    std::cout << "(0) Exit Program" << std::endl; 
    std::cout << "(1) No Effects" << std::endl;
    std::cout << "(2) Tremolo" << std::endl;
    std::cout << "(3) Delay" << std::endl;    
    std::cout << "(4) Reverb" << std::endl;            
    std::cout << "(5) Bitcrush" << std::endl;
    std::cout << "(6) Dirt" << std::endl;

    std::cout << "Enter the integer value of the effect you would like to apply: ";
    std::cin >> userChoice;

    choiceSelect(userChoice, effectChoice, validChoice);

    while (!validChoice){
        std::cout << "Enter a valid option: ";
        std::cin >> userChoice; 
        choiceSelect(userChoice, effectChoice, validChoice);
    }
    return true;
}