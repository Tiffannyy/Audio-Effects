/*
 * Engine.cpp
 *
 * Tiffany Liu, Nathaniel Kalaw
 * 24 February 2026
*/

#include <stdio.h>
#include "../include/Engine.h"
#include "../include/peripherals.h"
#include "../include/types.h"

extern AudioParams audioParams;

//#define ENGINE_DEBUG


// ============================================================
// [FUNCTIONS]
Engine::Engine(const char* inputDevice, const char* outputDevice){
    
    period = FRAMES_PER_BUFFER;
    buffer = FRAMES_PER_BUFFER * BUFFER_MULT;

    if (setupPCM(inputDevice, &inHandle,
                 SND_PCM_STREAM_CAPTURE,
                 2, audioParams.SAMPLE_RATE,
                 period, buffer) < 0){
        throw std::runtime_error("Failed to setup input PCM");
    }

    if (setupPCM(outputDevice, &outHandle,
                 SND_PCM_STREAM_PLAYBACK,
                 1, audioParams.SAMPLE_RATE,
                 period, buffer) < 0){
        throw std::runtime_error("Failed to setup output PCM");
    }

    snd_pcm_nonblock(inHandle, 1);
    snd_pcm_nonblock(outHandle, 1);
    
    #ifdef ENGINE_DEBUG
    printf("Engine successfully initialized input and output PCM devices.\n");
    #endif
}


Engine::~Engine(){
    stop();
    if (inHandle){
        snd_pcm_close(inHandle);
        inHandle = nullptr;
    }

    if (outHandle){
        snd_pcm_drain(outHandle);
        snd_pcm_close(outHandle);
        outHandle = nullptr;
    }
}


void Engine::start(){
    
    initData(userData, audioParams, effectChoice);
    
    // Initialize peripherals via wiringPi
    if (initializePeripherals() < 0) {
        printf("Failed to setup wiringPi peripherals.\n");
        exit(-1);
    }
    
    // Initialize threads for individual tasks
    running.store(true);
    peripheralThread = std::thread(&Engine::runPeripheralThread, this);
    //guiThread = std::thread(&Engine::runGUIThread, this);
    audioThread = std::thread(&Engine::runStreamLoop, this);
}


void Engine::stop(){
    // Join threads
    running.store(false);
    if (peripheralThread.joinable())
        peripheralThread.join();
    //guiThread.join();
    if (audioThread.joinable())
        audioThread.join();
    
    // Close periperhals
    closePeripherals();
}


void Engine::runPeripheralThread(void) {
    while (running.load(std::memory_order_relaxed)) {
        readPeripherals();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}


// void Engine::runGUIThread(void) {
//     system("clear");
//     while (running.load(std::memory_order_relaxed)) {
//         printEngineState();
//     }
// }


void Engine::runStreamLoop(){
    while(running.load())
    	stream(userData, audioParams, inHandle, outHandle, period, running);
}


// ============================================================
// [PARAMETER ADJUSTMENT]


void Engine::setVolume(float v) {
    audioParams.VOLUME.store(v);
}


void Engine::setMix(float v) {
    audioParams.MIX.store(v);
}


void Engine::adjustTremFreq(int inc){
    float v = audioParams.TREM_FREQ.load();
    v += inc * TREM_FREQ_INC;
    if (v >= TREM_FREQ_MAX) v = TREM_FREQ_MAX;
    if (v <= TREM_FREQ_MIN) v = TREM_FREQ_MIN;
    audioParams.TREM_FREQ.store(v);
}


void Engine::adjustTremDepth(int inc){
    float v = audioParams.TREM_DEPTH;
    v += inc * PARAM_INC;
    if (v >= PARAM_MAX) v = PARAM_MAX;
    if (v <= PARAM_MIN) v = PARAM_MIN;
    audioParams.TREM_DEPTH.store(v);
}


void Engine::adjustDelayMs(int inc){
    int v = audioParams.DELAY_MS;
    v += inc * DELAY_MS_INC;
    if (v >= DELAY_MS_MAX) v = DELAY_MS_MAX;
    if (v <= DELAY_MS_MIN) v = DELAY_MS_MIN;
    audioParams.DELAY_MS.store(v);
}


void Engine::adjustDelayFeedback(int inc){
    float v = audioParams.DELAY_FEEDBACK;
    v += inc * PARAM_INC;
    if (v >= PARAM_MAX) v = PARAM_MAX;
    if (v <= PARAM_MIN) v = PARAM_MIN;
    audioParams.DELAY_FEEDBACK.store(v);
}


void Engine::adjustReverbDecay(int inc){
    float v = audioParams.REVERB_DECAY;
    v += inc * PARAM_INC;
    if (v >= PARAM_MAX) v = PARAM_MAX;
    if (v <= PARAM_MIN) v = PARAM_MIN;
    audioParams.REVERB_DECAY.store(v);
}


void Engine::adjustBitcrushRate(int inc){
    int v = audioParams.BITCRUSH_RATE;
    v += inc * BITCRUSH_RATE_INC;
    if (v >= BITCRUSH_RATE_MAX) v = BITCRUSH_RATE_MAX;
    if (v <= BITCRUSH_RATE_MIN) v = BITCRUSH_RATE_MIN;
    audioParams.BITCRUSH_RATE.store(v);
}


void Engine::adjustBitcrushDepth(int inc){
    int v = audioParams.BITCRUSH_DEPTH;
    v += inc;
    if (v >= BITCRUSH_DEPTH_MAX) v = BITCRUSH_DEPTH_MAX;
    if (v <= BITCRUSH_DEPTH_MIN) v = BITCRUSH_DEPTH_MIN;
    audioParams.BITCRUSH_DEPTH.store(v);
}


void Engine::adjustOdDrive(int inc){
    float v = audioParams.OD_DRIVE;
    v += inc * PARAM_INC;
    if (v >= PARAM_MAX) v = PARAM_MAX;
    if (v <= PARAM_MIN) v = PARAM_MIN;
    audioParams.OD_DRIVE.store(v);
}


void Engine::adjustOdTone(int inc){
    float v = audioParams.OD_TONE;
    v += inc * PARAM_INC;
    if (v >= PARAM_MAX) v = PARAM_MAX;
    if (v <= PARAM_MIN) v = PARAM_MIN;
    audioParams.OD_TONE.store(v);
}


void Engine::adjustDistDrive(int inc){
    float v = audioParams.DIST_DRIVE;
    v += inc * PARAM_INC;
    if (v >= PARAM_MAX) v = PARAM_MAX;
    if (v <= PARAM_MIN) v = PARAM_MIN;
    audioParams.DIST_DRIVE.store(v);
}


void Engine::adjustDistTone(int inc){
    float v = audioParams.DIST_TONE;
    v += inc * PARAM_INC;
    if (v >= PARAM_MAX) v = PARAM_MAX;
    if (v <= PARAM_MIN) v = PARAM_MIN;
    audioParams.DIST_TONE.store(v);
}

void Engine::adjustFuzzDrive(int inc){
    float v = audioParams.FUZZ_DRIVE;
    v += inc * PARAM_INC;
    if (v >= PARAM_MAX) v = PARAM_MAX;
    if (v <= PARAM_MIN) v = PARAM_MIN;
    audioParams.FUZZ_DRIVE.store(v);
}

void Engine::adjustFuzzTone(int inc){
    float v = audioParams.FUZZ_TONE;
    v += inc * PARAM_INC;
    if (v >= PARAM_MAX) v = PARAM_MAX;
    if (v <= PARAM_MIN) v = PARAM_MIN;
    audioParams.FUZZ_TONE.store(v);
}


void Engine::updateEffectChoice(void){
    effectChoice.norm.store(false);
    effectChoice.trem.store(false);
    effectChoice.delay.store(false);
    effectChoice.reverb.store(false);
    effectChoice.bitcrush.store(false);
    effectChoice.overdrive.store(false);
    effectChoice.distortion.store(false);
    effectChoice.fuzz.store(false);

    switch (effectSelection){
        case NO_EFFECT:
            effectChoice.norm.store(true);
            break;
        case TREMOLO:
            effectChoice.trem.store(true);
            break;
        case DELAY:
            effectChoice.delay.store(true);
            break;
        case REVERB:
            effectChoice.reverb.store(true);
            break;
        case BITCRUSH:
            effectChoice.bitcrush.store(true);
            break;
        case OVERDRIVE:
            effectChoice.overdrive.store(true);
            break;
        case DISTORTION:
            effectChoice.distortion.store(true);
            break;
        case FUZZ:
            effectChoice.fuzz.store(true);
            break;
        default:
            effectChoice.norm.store(true);
            break;
    }
}


// ============================================================
// [READING PERIPHERALS]
UIState Engine::getUIState(){
    UIState s;
    s.menuMode = menuMode;
    s.effectSel = effectSelection;
    s.tremSel = audioParamSelection.TREMOLO;
    s.delaySel = audioParamSelection.DELAY;
    s.reverbSel = audioParamSelection.REVERB;
    s.bitcrushSel = audioParamSelection.BITCRUSH;
    s.odSel = audioParamSelection.OVERDRIVE;
    s.distSel = audioParamSelection.DISTORTION;
    s.fuzzSel = audioParamSelection.FUZZ;
    return s;
}
std::map<std::string, float> Engine::getParams(){
    return {
        {"volume", (float)audioParams.VOLUME.load()},
        {"mix", (float)audioParams.MIX.load()},
        {"trem_freq", (float)audioParams.TREM_FREQ.load()},
        {"trem_depth", (float)audioParams.TREM_DEPTH.load()},
        {"delay_ms", (float)audioParams.DELAY_MS.load()},
        {"delay_feedback", (float)audioParams.DELAY_FEEDBACK.load()},
        {"reverb_decay", (float)audioParams.REVERB_DECAY.load()},
        {"bitcrush_rate", (float)audioParams.BITCRUSH_RATE.load()},
        {"bitcrush_depth", (float)audioParams.BITCRUSH_DEPTH.load()},
        {"od_drive", (float)audioParams.OD_DRIVE.load()},
        {"od_tone", (float)audioParams.OD_TONE.load()},
        {"dist_drive", (float)audioParams.DIST_DRIVE.load()},
        {"dist_tone", (float)audioParams.DIST_TONE.load()},
        {"fuzz_drive", (float)audioParams.FUZZ_DRIVE.load()},
        {"fuzz_tone", (float)audioParams.FUZZ_TONE.load()}
    };
}


void Engine::readPeripherals(void) {
    readPotentiometers();
    readEncoder();
    readButton();
}


void Engine::readPotentiometers(void) {
    setVolume(peripheralData.VOLUME_VALUE);
    setMix(peripheralData.MIX_VALUE);
}

    
void Engine::readEncoder(void) {
    // Detect encoder turns
    if (peripheralData.ENCODER_TURNED) {
        
        // Effect selection
        if (menuMode == EFFECT_SELECTION_MODE) {
            int index = effectSelection;
            index += peripheralData.ENCODER_TURNED;
            if (index < 0) index = 0;
            if (index > EFFECT_SELECTION_MAX) index = EFFECT_SELECTION_MAX;
            effectSelection = static_cast<EffectSelection>(index);
        }
        
        // Effect parameter selection
        else if (menuMode == AUDIO_PARAM_SELECTION_MODE) {
            switch (effectSelection) {
		case NO_EFFECT:
		    break;
                case TREMOLO: {
                    int index = audioParamSelection.TREMOLO;
                    index += peripheralData.ENCODER_TURNED;
                    if (index < 0) index = 0;
                    if (index > TREMOLO_SELECTION_MAX) index = TREMOLO_SELECTION_MAX;
                    audioParamSelection.TREMOLO = static_cast<TremoloSelection>(index);
                    break;
                }

                case DELAY: {
                    int index = audioParamSelection.DELAY;
                    index += peripheralData.ENCODER_TURNED;
                    if (index < 0) index = 0;
                    if (index > DELAY_SELECTION_MAX) index = DELAY_SELECTION_MAX;
                    audioParamSelection.DELAY = static_cast<DelaySelection>(index);
                    break;
                }
                
                case REVERB: {
                    int index = audioParamSelection.REVERB;
                    index += peripheralData.ENCODER_TURNED;
                    if (index < 0) index = 0;
                    if (index > REVERB_SELECTION_MAX) index = REVERB_SELECTION_MAX;
                    audioParamSelection.REVERB = static_cast<ReverbSelection>(index);
                    break;
                }
                
                case BITCRUSH: {
                    int index = audioParamSelection.BITCRUSH;
                    index += peripheralData.ENCODER_TURNED;
                    if (index < 0) index = 0;
                    if (index > BITCRUSH_SELECTION_MAX) index = BITCRUSH_SELECTION_MAX;
                    audioParamSelection.BITCRUSH = static_cast<BitcrushSelection>(index);
                    break;
                }
                
                case OVERDRIVE: {
                    int index = audioParamSelection.OVERDRIVE;
                    index += peripheralData.ENCODER_TURNED;
                    if (index < 0) index = 0;
                    if (index > OVERDRIVE_SELECTION_MAX) index = OVERDRIVE_SELECTION_MAX;
                    audioParamSelection.OVERDRIVE = static_cast<OverdriveSelection>(index);
                    break;
                }
                
                case DISTORTION: {
                    int index = audioParamSelection.DISTORTION;
                    index += peripheralData.ENCODER_TURNED;
                    if (index < 0)  index = 0;
                    if (index > DISTORTION_SELECTION_MAX) index = DISTORTION_SELECTION_MAX;
                    audioParamSelection.DISTORTION = static_cast<DistortionSelection>(index);
                    break;
                }
                
                case FUZZ: {
                    int index = audioParamSelection.FUZZ;
                    index += peripheralData.ENCODER_TURNED;
                    if (index < 0)
                        index = 0;
                    if (index > FUZZ_SELECTION_MAX)
                        index = FUZZ_SELECTION_MAX;
                    audioParamSelection.FUZZ = static_cast<FuzzSelection>(index);
                    break;
                }

            } // [END SWITCH]
        } // [END PARAMETER SELECTION]

        // Adjusting parameter values
        else if (menuMode == AUDIO_PARAM_VALUE_MODE) {      
            int inc = peripheralData.ENCODER_TURNED;    
            
            // Increment or decrement the effect parameters in the audioParams structure  
            switch (effectSelection){
		case NO_EFFECT:
		    break;
		case TREMOLO: {
                    switch (audioParamSelection.TREMOLO){
                        case TREMOLO_FREQ:  adjustTremFreq(inc); break;
                        case TREMOLO_DEPTH: adjustTremDepth(inc); break;
			case TREMOLO_BACK: break;
                        //case TREMOLO_PHASE: /*... */ break;
		    }break;
		}

                case DELAY: {
                    switch (audioParamSelection.DELAY){
                        case DELAY_MS:       adjustDelayMs(inc); break;
                        case DELAY_FEEDBACK: adjustDelayFeedback(inc); break;
			case DELAY_BACK: break;
	   	     }break;
		}
               
                case REVERB: {
                    switch (audioParamSelection.REVERB){
                        //case REVERB_TAPS: /*... */ break;
                        case REVERB_DECAY: adjustReverbDecay(inc); break;
		        case REVERB_BACK: break;
		    }break;
		}
                
		case BITCRUSH: {
                    switch (audioParamSelection.BITCRUSH){
                        case BITCRUSH_RATE: adjustBitcrushRate(inc); break;
                        case BITCRUSH_DEPTH: adjustBitcrushDepth(inc); break;
			case BITCRUSH_BACK: break;		    
		    }break;
		}
                
                case OVERDRIVE: {
                    switch (audioParamSelection.OVERDRIVE){
                        case OVERDRIVE_DRIVE: adjustOdDrive(inc); break;
                        case OVERDRIVE_TONE: adjustOdTone(inc); break;
			case OVERDRIVE_BACK: break;
                    }break;
		}

                case DISTORTION: {
                    switch (audioParamSelection.DISTORTION){
                        case DISTORTION_DRIVE: adjustDistDrive(inc); break;
                        case DISTORTION_TONE: adjustDistTone(inc); break;
			case DISTORTION_BACK: break;
	    	    }break;
		}

                case FUZZ: {
                    switch (audioParamSelection.FUZZ){
                        case FUZZ_DRIVE: adjustFuzzDrive(inc); break;
                        case FUZZ_TONE: adjustFuzzTone(inc); break;
			case FUZZ_BACK: break;
                    }break;
		}
	    }// [END ADJUSTING PARAMETERS]
	} 
        // Reset encoder turned flag
        peripheralData.ENCODER_TURNED = 0;
        
    } // [END IF]
    
} // [END READ ENCODER]


void Engine::readButton(void) {
    
    // Detect button push to change modes
    if (peripheralData.ENCODER_PRESSED) {
        
        // Effect selection
        if (menuMode == EFFECT_SELECTION_MODE){
            updateEffectChoice();
            if (effectSelection != NO_EFFECT)
                menuMode = AUDIO_PARAM_SELECTION_MODE;
        }
        
        // Effect parameter selection
        else if (menuMode == AUDIO_PARAM_SELECTION_MODE) {
            
            // Must check if "go back" or parameter was selected
            switch (effectSelection) {
              	case NO_EFFECT: break;

                case TREMOLO: {
                    if (audioParamSelection.TREMOLO == TREMOLO_BACK) {
                        audioParamSelection.TREMOLO = static_cast<TremoloSelection>(0);
                        menuMode = EFFECT_SELECTION_MODE;
                    }
                    else
                        menuMode = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                    
                case DELAY: {
                    if (audioParamSelection.DELAY == DELAY_BACK) {
                        audioParamSelection.DELAY = static_cast<DelaySelection>(0);
                        menuMode = EFFECT_SELECTION_MODE;
                    }
                    else
                        menuMode = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                
                case REVERB: {
                    if (audioParamSelection.REVERB == REVERB_BACK) {
                        audioParamSelection.REVERB = static_cast<ReverbSelection>(0);
                        menuMode = EFFECT_SELECTION_MODE;
                    }
                    else
                        menuMode = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                
                case BITCRUSH: {
                    if (audioParamSelection.BITCRUSH == BITCRUSH_BACK) {
                        audioParamSelection.BITCRUSH = static_cast<BitcrushSelection>(0);
                        menuMode = EFFECT_SELECTION_MODE;
                    }
                    else
                        menuMode = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                
                case OVERDRIVE: {
                    if (audioParamSelection.OVERDRIVE == OVERDRIVE_BACK) {
                        audioParamSelection.OVERDRIVE = static_cast<OverdriveSelection>(0);
                        menuMode = EFFECT_SELECTION_MODE;
                    }
                    else
                        menuMode = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                
                case DISTORTION: {
                    if (audioParamSelection.DISTORTION == DISTORTION_BACK) {
                        audioParamSelection.DISTORTION = static_cast<DistortionSelection>(0);
                        menuMode = EFFECT_SELECTION_MODE;
                    }
                    else
                        menuMode = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
                
                case FUZZ: {
                    if (audioParamSelection.FUZZ == FUZZ_BACK) {
                        audioParamSelection.FUZZ = static_cast<FuzzSelection>(0);
                        menuMode = EFFECT_SELECTION_MODE;
                    }
                    else
                        menuMode = AUDIO_PARAM_VALUE_MODE;
                    break;
                }
            } // [END SWITCH]
        } // [END PARAMETER SELECTION]
        
        // Adjusting parameter value
        else if (menuMode == AUDIO_PARAM_VALUE_MODE)
            menuMode = AUDIO_PARAM_SELECTION_MODE;
        
        // Reset button pressed flag
        peripheralData.ENCODER_PRESSED = false;
        
    } // [END IF]
    
} // [END READ BUTTON]


// for debugging
void Engine::printEngineState(void) {
    
    printf("\033[%d;%dH", 0, 0);
    
    switch (menuMode) {
        case EFFECT_SELECTION_MODE:      printf("\e[0;93mSelecting effect   \e[0m\n"); break;
        case AUDIO_PARAM_SELECTION_MODE: printf("\e[0;93mSelecting parameter\e[0m\n"); break;
        case AUDIO_PARAM_VALUE_MODE:     printf("\e[0;92mAdjusting parameter\e[0m\n"); break;
    }
    
    printf("\e[0;96mCurr selection: \e[0m");
    switch (effectSelection) {
        case NO_EFFECT: 
            printf("No effect");
            break;
        case TREMOLO:    
            printf("Tremolo | "); 
            switch (audioParamSelection.TREMOLO) {
                case TREMOLO_FREQ:  printf("Freq: %.2f", audioParams.TREM_FREQ.load()); break;
                case TREMOLO_DEPTH: printf("Depth: %.2f", audioParams.TREM_DEPTH.load()); break;
                case TREMOLO_BACK:  printf("Back"); break;
            }
            break;
        case DELAY:      
            printf("Delay | "); 
            switch (audioParamSelection.DELAY) {
                case DELAY_MS:       printf("Ms: %d", audioParams.DELAY_MS.load()); break;
                case DELAY_FEEDBACK: printf("Feedback: %.2f", audioParams.DELAY_FEEDBACK.load()); break;
                case DELAY_BACK:     printf("Back"); break;
            }
            break;
        case REVERB:     
            printf("Reverb | "); 
            switch (audioParamSelection.REVERB) {
                case REVERB_DECAY: printf("Decay: %.2f", audioParams.REVERB_DECAY.load()); break;
                case REVERB_BACK:  printf("Back"); break;
            }
            break;
        case BITCRUSH:   
            printf("Bitcrush | "); 
            switch (audioParamSelection.BITCRUSH) {
                case BITCRUSH_RATE:  printf("Rate: %d", audioParams.BITCRUSH_RATE.load()); break;
                case BITCRUSH_DEPTH: printf("Depth: %d", audioParams.BITCRUSH_DEPTH.load()); break;
                case BITCRUSH_BACK:  printf("Back"); break;
            }
            break;
        case OVERDRIVE:  
            printf("Overdrive | "); 
            switch (audioParamSelection.OVERDRIVE) {
                case OVERDRIVE_DRIVE:  printf("Drive: %.2f", audioParams.OD_DRIVE.load()); break;
                case OVERDRIVE_TONE:   printf("Tone: %.2f", audioParams.OD_TONE.load()); break;
                case OVERDRIVE_BACK:   printf("Back"); break;
            }
            break;
        case DISTORTION: 
            printf("Distortion | "); 
            switch (audioParamSelection.DISTORTION) {
                case DISTORTION_DRIVE:  printf("Drive: %.2f", audioParams.DIST_DRIVE.load()); break;
                case DISTORTION_TONE:   printf("Tone: %.2f", audioParams.DIST_TONE.load()); break;
                case DISTORTION_BACK:   printf("Back"); break;
            }
            break;
        case FUZZ:       
            printf("Fuzz | "); 
            switch (audioParamSelection.FUZZ) {
                case FUZZ_DRIVE:  printf("Drive: %.2f", audioParams.FUZZ_DRIVE.load()); break;
                case FUZZ_TONE:   printf("Tone: %.2f", audioParams.FUZZ_TONE.load()); break;
                case FUZZ_BACK:   printf("Back"); break;
            }
            break;
    }
    
    printf("                             \n");
    printf("Vol: %.2f | Mix: %.2f\n", audioParams.VOLUME.load(), audioParams.MIX.load());

}
