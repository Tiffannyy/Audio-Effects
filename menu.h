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
#include <stdlib.h>

#include "types.h"

using namespace std;

// Menu function
void menuFunction(EffectChoices& effectChoice){
    bool validChoice = false;
    char userChoice;
    cout << "*------ Audio Effects Program ------*\n" << "Effects Options:\n";
    cout << "(0) Exit Program" << endl; 
    cout << "(1) No Effects" << endl;
    cout << "(2) Tremolo" << endl;
    cout << "(3) Delay" << endl;                  
//    cout << "(4) Volume Adjuster" << endl;
    cout << "Enter the integer value of the effect you would like to apply: ";
    cin >> userChoice;
    
    switch (userChoice){
            case '0':
                cout << "Ending program..." << endl;
                exit(0);
                break;
            case '1':                       //no effect
                effectChoice.norm = true;
                validChoice = true;
                break;
            case '2':                       //tremolo
                effectChoice.trem = true;
                validChoice = true;
                break;
            case '3':
                effectChoice.delay = true;
                validChoice = true;
                break;
            default:
                break;
        }
    
    while (!validChoice){
    cout << "Enter a valid option: ";
    cin >> userChoice; 
        switch (userChoice){
            case '0':
                cout << "Ending program..." << endl;
                exit(0);
                break;
            case '1':                       //no effect
                effectChoice.norm = true;
                validChoice = true;
                break;
            case '2':                       //tremolo
                effectChoice.trem = true;
                validChoice = true;
                break;
            case '3':
                effectChoice.delay = true;
                validChoice = true;
                break;
            default:
                break;
        }
      
    }
    return;
}