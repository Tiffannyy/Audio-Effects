/*
 * menu.cpp
 *
 * Tiffany Liu
 * 5 February 2026
 * 
 * Description: Implementation of menu function
*/

#include <iostream>

// Choice function
void choiceSelect(char choice, EffectChoices &effectChoice, bool &validChoice, bool &exitFlag){
    switch (choice){
        case '0':
            std::cout << "Ending program..." << std::endl;
	    validChoice = true;
	    exitFlag = true;
	    break;
        case '1':
            effectChoice.norm = true;
            validChoice = true;
            break;
        case '2':
            effectChoice.trem = true;
            validChoice = true;
            break;
        case '3':
            effectChoice.delay = true;
            validChoice = true;
            break;
        case '4':
            effectChoice.reverb = true;
            validChoice = true;
            break;
        case '5':
            effectChoice.bitcrush = true;
            validChoice = true;
            break;
        case '6':
            effectChoice.overdrive = true;
            validChoice = true;
            break;
        case '7':
            effectChoice.distortion = true;
            validChoice = true;
            break;
        case '8':
            effectChoice.fuzz = true;
            validChoice = true;
            break;
        default:
            break;
    }
}

// Menu function
bool menuFunction(EffectChoices &effectChoice){
    bool validChoice = false;
    bool exitFlag = false;
    char userChoice;
    std::cout << "*------ Audio Effects Program ------*\n" << "Effects Options:\n";
    std::cout << "(0) Exit Program" << std::endl; 
    std::cout << "(1) No Effects" << std::endl;
    std::cout << "(2) Tremolo" << std::endl;
    std::cout << "(3) Delay" << std::endl;    
    std::cout << "(4) Reverb" << std::endl;            
    std::cout << "(5) Bitcrush" << std::endl;
    std::cout << "(6) Overdrive" << std::endl;
    std::cout << "(7) Distortion" << std::endl;
    std::cout << "(8) Fuzz" << std::endl;

    std::cout << "Enter the integer value of the effect you would like to apply: ";
    std::cin >> userChoice;

    choiceSelect(userChoice, effectChoice, validChoice, exitFlag);
   
    while (!validChoice){
        std::cout << "Enter a valid option: ";
        std::cin >> userChoice; 
        choiceSelect(userChoice, effectChoice, validChoice, exitFlag);
    }

    if (exitFlag)
	    return false;
    return true;
}