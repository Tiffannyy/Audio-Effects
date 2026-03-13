/*
 * Engine.cpp
 *
 * Tiffany Liu
 * 9 February 2026
*/

#include <stdio.h>
#include "Engine_test.h"
#include "peripherals.h"


// ============================================================
// [VARIABLES]


MenuMode            MENU_MODE        = EFFECT_SELECTION_MODE;
EffectSelection     EFFECT_SELECTION = NO_EFFECT;
AudioParamSelection AUDIO_PARAM_SELECTION;


// ============================================================
// [FUNCTIONS]


void readPeripherals(void) {
    
    // Read and apply volume value
    // ...
    
    // Read and apply mix value
    // ...
    

    // ----------
    

    // Detect encoder turns
    if (PERIPHERAL_DATA.encoderTurned) {
        
        // Effect selection
        if (MENU_MODE == EFFECT_SELECTION_MODE) {
            int index = EFFECT_SELECTION;
            index += PERIPHERAL_DATA.encoderTurned;
            if (index < 0) index = 0;
            if (index > EFFECT_SELECTION_MAX) index = EFFECT_SELECTION_MAX;
            EFFECT_SELECTION = static_cast<EffectSelection>(index);
        }
        
        // Effect parameter selection
        else if (MENU_MODE == AUDIO_PARAM_SELECTION_MODE) {
            switch (EFFECT_SELECTION) {
                
                case TREMOLO: {
                    int index = AUDIO_PARAM_SELECTION.TREMOLO;
                    index += PERIPHERAL_DATA.encoderTurned;
                    if (index < 0) index = 0;
                    if (index > TREMOLO_SELECTION_MAX) index = TREMOLO_SELECTION_MAX;
                    AUDIO_PARAM_SELECTION.TREMOLO = static_cast<TremoloSelection>(index);
                    break;
                }

                case DELAY: {
                    int index = AUDIO_PARAM_SELECTION.DELAY;
                    index += PERIPHERAL_DATA.encoderTurned;
                    if (index < 0) index = 0;
                    if (index > DELAY_SELECTION_MAX) index = DELAY_SELECTION_MAX;
                    AUDIO_PARAM_SELECTION.DELAY = static_cast<DelaySelection>(index);
                    break;
                }
                
                case REVERB: {
                    int index = AUDIO_PARAM_SELECTION.REVERB;
                    index += PERIPHERAL_DATA.encoderTurned;
                    if (index < 0) index = 0;
                    if (index > REVERB_SELECTION_MAX) index = REVERB_SELECTION_MAX;
                    AUDIO_PARAM_SELECTION.REVERB = static_cast<ReverbSelection>(index);
                    break;
                }
                
                case BITCRUSH: {
                    int index = AUDIO_PARAM_SELECTION.BITCRUSH;
                    index += PERIPHERAL_DATA.encoderTurned;
                    if (index < 0) index = 0;
                    if (index > BITCRUSH_SELECTION_MAX) index = BITCRUSH_SELECTION_MAX;
                    AUDIO_PARAM_SELECTION.BITCRUSH = static_cast<BitcrushSelection>(index);
                    break;
                }
                
                case OVERDRIVE: {
                    int index = AUDIO_PARAM_SELECTION.OVERDRIVE;
                    index += PERIPHERAL_DATA.encoderTurned;
                    if (index < 0) index = 0;
                    if (index > OVERDRIVE_SELECTION_MAX) index = OVERDRIVE_SELECTION_MAX;
                    AUDIO_PARAM_SELECTION.OVERDRIVE = static_cast<OverdriveSelection>(index);
                    break;
                }
                
                case DISTORTION: {
                    int index = AUDIO_PARAM_SELECTION.DISTORTION;
                    index += PERIPHERAL_DATA.encoderTurned;
                    if (index < 0)  index = 0;
                    if (index > DISTORTION_SELECTION_MAX) index = DISTORTION_SELECTION_MAX;
                    AUDIO_PARAM_SELECTION.DISTORTION = static_cast<DistortionSelection>(index);
                    break;
                }
                
                case FUZZ: {
                    int index = AUDIO_PARAM_SELECTION.FUZZ;
                    index += PERIPHERAL_DATA.encoderTurned;
                    if (index < 0)
                        index = 0;
                    if (index > FUZZ_SELECTION_MAX)
                        index = FUZZ_SELECTION_MAX;
                    AUDIO_PARAM_SELECTION.FUZZ = static_cast<FuzzSelection>(index);
                    break;
                }

            } // [END SWITCH]
        } // [END EFFECT PARAMETER SELECTION]

        // Adjusting parameter values
        else if (MENU_MODE == AUDIO_PARAM_VALUE_MODE) {

            /*  This is where you would increment or decrement the effect parameters
             *  in the audioParams structure.
             *
             *  Do a switch/if-else for EFFECT_SELECTION and AUDIO_PARAM_SELECTION.<effect>
             *  to determine which parameter you want to adjust.
             */

            switch (EFFECT_SELECTION) {
                
                case TREMOLO: {
                    switch (AUDIO_PARAM_SELECTION.TREMOLO) {
                        case TREMOLO_FREQ: /*... */ break;
                        case TREMOLO_DEPTH: /*... */ break;
                        case TREMOLO_PHASE: /*... */ break;
                    } break;
                }

                case DELAY: {
                    switch (AUDIO_PARAM_SELECTION.DELAY) {
                        case DELAY_MS: /*... */ break;
                        case DELAY_FEEDBACK: /*... */ break;
                    } break;
                }
                
                case REVERB: {
                    switch (AUDIO_PARAM_SELECTION.REVERB) {
                        case REVERB_TAPS: /*... */ break;
                        case REVERB_DECAY: /*... */ break;
                    } break;
                }
                
                case BITCRUSH: {
                    switch (AUDIO_PARAM_SELECTION.BITCRUSH) {
                        case BITCRUSH_DOWNSAMPLE: /*... */ break;
                        case BITCRUSH_DEPTH: /*... */ break;
                    } break;
                }
                
                case OVERDRIVE: {
                    switch (AUDIO_PARAM_SELECTION.OVERDRIVE) {
                        case OVERDRIVE_DRIVE: /*... */ break;
                        case OVERDRIVE_TONE: /*... */ break;
                    } break;
                }
                
                case DISTORTION: {
                    switch (AUDIO_PARAM_SELECTION.DISTORTION) {
                        case DISTORTION_DRIVE: /*... */ break;
                        case DISTORTION_TONE: /*... */ break;
                    } break;
                }
                
                case FUZZ: {
                    switch (AUDIO_PARAM_SELECTION.FUZZ) {
                        case FUZZ_DRIVE: /*... */ break;
                        case FUZZ_TONE: /*... */ break;
                    } break;
                }
            } // [END SWITCH]
        } // [END ADJUSTING PARAMETERS]
        
        // Reset encoder turned flag
        PERIPHERAL_DATA.encoderTurned = 0;
        
    } // [END ENCODER TURNING]
    

    // ----------
    

    // Detect button push to change modes
    if (PERIPHERAL_DATA.encoderPressed) {
        
        // Effect selection
        if (MENU_MODE == EFFECT_SELECTION_MODE && EFFECT_SELECTION != NO_EFFECT)
            MENU_MODE = AUDIO_PARAM_SELECTION_MODE;
        
        // Effect parameter selection
        else if (MENU_MODE == AUDIO_PARAM_SELECTION_MODE) {
            switch (EFFECT_SELECTION) {
                
                /* Must check if "go back" or parameter was selected */
                
                case TREMOLO: {
                    if (AUDIO_PARAM_SELECTION.TREMOLO == TREMOLO_BACK)
                        MENU_MODE = EFFECT_SELECTION_MODE;
                    else
                        MENU_MODE = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                    
                case DELAY: {
                    if (AUDIO_PARAM_SELECTION.DELAY == DELAY_BACK)
                        MENU_MODE = EFFECT_SELECTION_MODE;
                    else
                        MENU_MODE = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                
                case REVERB: {
                    if (AUDIO_PARAM_SELECTION.REVERB == REVERB_BACK)
                        MENU_MODE = EFFECT_SELECTION_MODE;
                    else
                        MENU_MODE = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                
                case BITCRUSH: {
                    if (AUDIO_PARAM_SELECTION.BITCRUSH == BITCRUSH_BACK)
                        MENU_MODE = EFFECT_SELECTION_MODE;
                    else
                        MENU_MODE = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                
                case OVERDRIVE: {
                    if (AUDIO_PARAM_SELECTION.OVERDRIVE == OVERDRIVE_BACK)
                        MENU_MODE = EFFECT_SELECTION_MODE;
                    else
                        MENU_MODE = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                
                case DISTORTION: {
                    if (AUDIO_PARAM_SELECTION.DISTORTION == DISTORTION_BACK)
                        MENU_MODE = EFFECT_SELECTION_MODE;
                    else
                        MENU_MODE = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                
                case FUZZ: {
                    if (AUDIO_PARAM_SELECTION.FUZZ == FUZZ_BACK)
                        MENU_MODE = EFFECT_SELECTION_MODE;
                    else
                        MENU_MODE = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
            } // [END SWITCH]
        } // [END EFFECT PARAMETER SELECTION]
        
        // Adjusting parameter value
        else if (MENU_MODE == AUDIO_PARAM_VALUE_MODE)
            MENU_MODE = AUDIO_PARAM_SELECTION_MODE;
        
        // Reset button pressed flag
        PERIPHERAL_DATA.encoderPressed = false;
        
    } // [END BUTTON DETECTION]
    

}; // [END READ PERIPHERALS]




