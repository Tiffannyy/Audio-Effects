/*
 * main.cpp
 * DSP Program
 *
 * Tiffany Liu
 * 5 February 2026
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

#include "../include/menu.h"
#include "../include/callback.h"
#include "../include/types.h"

using namespace std;

// User Definitions
#define FRAMES_PER_BUFFER 256
#define BUFFER_MULT 4
const bool DEBUG = 0;

const char* DEVICE_NAME = "hw:0,0";

// function prototypes
int setupPCM(const char* device, snd_pcm_t** handle, snd_pcm_stream_t stream,
        unsigned int channels, unsigned int rate, snd_pcm_uframes_t period,
        snd_pcm_uframes_t buffer);

void initData(RtUserData &ud, AudioParams &audioParams, EffectChoices &effectChoice);

void resetData(RtUserData &ud);

void stream(RtUserData &ud, AudioParams &audioParams,
		EffectChoices &effectChoice,
	       	snd_pcm_t *inHandle, snd_pcm_t *outHandle,
		snd_pcm_uframes_t period);


// main function
int main(){
    // Declare stream parameters
    snd_pcm_t *inHandle, *outHandle;
    
    // setup PCM device
    snd_pcm_uframes_t period = FRAMES_PER_BUFFER;
    snd_pcm_uframes_t buffer = FRAMES_PER_BUFFER * BUFFER_MULT;

    if (setupPCM(DEVICE_NAME, &inHandle, SND_PCM_STREAM_CAPTURE, 2, 44100, period, buffer) < 0) return 1;
    if (setupPCM(DEVICE_NAME, &outHandle, SND_PCM_STREAM_PLAYBACK, 2, 44100, period, buffer) < 0) return 1;

    snd_pcm_nonblock(inHandle, 1);
    snd_pcm_nonblock(outHandle, 1);
   
    initData(userData, audioParams, effectChoice);

    // begin main loop
    while (true) {
        bool keepRunning = menuFunction(effectChoice);
        if (!keepRunning) break;
        stream(userData, audioParams, effectChoice, inHandle, outHandle, period);
    }
}
