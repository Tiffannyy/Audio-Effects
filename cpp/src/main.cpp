/*
 * main.cpp
 * DSP Program
 *
 * Tiffany Liu
 * 24 February 2026
 * 
 * Description: Implementation of main program
*/

//Libraries
#include <cstdio>
#include <cstdlib>
#include <alsa/asoundlib.h>
#include <limits>
#include <string>
#include <poll.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>

//#include "../include/menu.h"
#include "../include/callback.h"
#include "../include/types.h"
#include "../include/audio.h"
#include "../include/Engine.h"

using namespace std;
const char* INPUT_DEVICE_NAME = "hw:3,0";
const char* OUTPUT_DEVICE_NAME = "hw:2,0";


// main function
int main(){
    
    Engine engine(INPUT_DEVICE_NAME, OUTPUT_DEVICE_NAME); 
    engine.start();
    
    while(1) {
        
    }
    
    /*
    // Declare stream parameters
    snd_pcm_t *inHandle, *outHandle;
    AudioParams audioParams;
    EffectChoices effectChoice;
    RtUserData userData;
    
    // setup PCM device
    snd_pcm_uframes_t period = FRAMES_PER_BUFFER;
    snd_pcm_uframes_t buffer = FRAMES_PER_BUFFER * BUFFER_MULT;

    if (setupPCM(INPUT_DEVICE_NAME, &inHandle, SND_PCM_STREAM_CAPTURE, 2, 44100, period, buffer) < 0) return 1;
    if (setupPCM(OUTPUT_DEVICE_NAME, &outHandle, SND_PCM_STREAM_PLAYBACK, 2, 44100, period, buffer) < 0) return 1;

    snd_pcm_nonblock(inHandle, 1);
    snd_pcm_nonblock(outHandle, 1);
   
    initData(userData, audioParams, effectChoice);

    // begin main loop
    while (true) {
        bool keepRunning = menuFunction(effectChoice);
        if (!keepRunning) break;
        stream(userData, audioParams, effectChoice, inHandle, outHandle, period);
    }
    
    */
}





