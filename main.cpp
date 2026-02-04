/*
 * main.cpp
 * DSP Program
 *
 * Tiffany Liu
 *
 * Description: Main file for DSP program.
*/

//Libraries
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <limits>
#include <string>
#include <iostream>
#include <poll.h>

#include "menu.h"
#include "realtime_callback.h"
#include "types.h"

using namespace std;

// User Definitions
#define FRAMES_PER_BUFFER 256
const bool DEBUG = 0;

// setup and open pcm
int setupPCM(const char* device, snd_pcm_t** handle, snd_pcm_stream_t stream,
	     unsigned int channels, unsigned int rate,
	     snd_pcm_uframes_t period, snd_pcm_uframes_t buffer){
    
    int err = snd_pcm_open(handle, device, stream, 0);
    if (err < 0) {
        fprintf(stderr, "Error opening PCM device: %s\n", snd_strerror(err));
        return err;
    }

    snd_pcm_hw_params_t* params;

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(*handle, params);
    snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(*handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(*handle, params, channels);
    snd_pcm_hw_params_set_rate(*handle, params, rate, 0);
    snd_pcm_hw_params_set_period_size_near(*handle, params, &period, 0);
    snd_pcm_hw_params_set_buffer_size_near(*handle, params, &buffer);
    err = snd_pcm_hw_params(*handle, params);
    if (err < 0) {
        fprintf(stderr, "Error setting input PCM parameters: %s\n", snd_strerror(err));
        snd_pcm_close(*handle);
        return err;
    }
    snd_pcm_prepare(*handle);

    if (DEBUG){
        snd_pcm_uframes_t actual_period, actual_buffer;
        snd_pcm_hw_params_get_period_size(params, &actual_period, 0);
        snd_pcm_hw_params_get_buffer_size(params, &actual_buffer);
        printf("Period: %lu, Buffer: %lu\n", actual_period, actual_buffer);
    }

    // set sw params
    snd_pcm_sw_params_t* sw_params;
    snd_pcm_sw_params_alloc(&sw_params);
    snd_pcm_sw_params_current(*handle, sw_params);
    // minimum # of frames for CPU to poll
    snd_pcm_sw_params_set_avail_min(*handle, sw_params, period);

    if (stream == SND_PCM_STREAM_PLAYBACK) {
        snd_pcm_sw_params_set_start_threshold(*handle, sw_params, period);
    } else {
        snd_pcm_sw_params_set_start_threshold(*handle, sw_params, 1);
    }

    err = snd_pcm_sw_params(*handle, sw_params);
    if (err < 0) {
        fprintf(stderr, "Error setting SW parameters: %s\n", snd_strerror(err));
        return err;
    }

    return 0;
}


// initialize data
void initData(RtUserData &ud, AudioParams &audioParams, EffectChoices &effectChoice){
    ud.params = &audioParams;
    ud.effects = &effectChoice;
 
    ud.tremIncrement = 2.0 * M_PI * audioParams.TREM_FREQ / (float)AudioParams::SAMPLE_RATE;
 
    ud.delaySize = max((float)1, AudioParams::DELAY_MS * (float)AudioParams::SAMPLE_RATE / 1000);
    ud.delayBuffer.assign(ud.delaySize, 0.0f);
    ud.delayIndex = 0;
 
    ud.reverbSize = AudioParams::SAMPLE_RATE;
    ud.reverbBuffer.assign(ud.reverbSize, 0.0f);
    float tapsMs[AudioParams::REVERB_TAPS] = {40, 50, 60, 80, 110};
    float gains[AudioParams::REVERB_TAPS] = {0.6f, 0.5f, 0.4f, 0.3f, 0.25f};
    for (int i = 0; i < AudioParams::REVERB_TAPS; i++){
        ud.reverbDelay[i] = tapsMs[i] * AudioParams::SAMPLE_RATE / 1000;
        ud.reverbGain[i] = gains[i];
	ud.reverbIndex[i] = 0;
    }
 
    ud.bitcrushCount = 0.0f;
    ud.bitcrushSample = 0.0f;
 
    return;
}


// reset DSP data
void resetData(RtUserData &ud){
    std::fill(ud.delayBuffer.begin(), ud.delayBuffer.end(), 0.0f);
    ud.delayIndex = 0;

    std::fill(ud.reverbBuffer.begin(), ud.reverbBuffer.end(), 0.0f);
    for (int i = 0; i < AudioParams::REVERB_TAPS; i++)
	ud.reverbIndex[i] = 0;   

    ud.bitcrushCount = 0.0f;
    ud.bitcrushSample = 0.0f;

    if (ud.params)
    	ud.params->tremPhase = 0.0f;
}


// main function
int main(){
    // Declare stream parameters
    snd_pcm_t *inHandle, *outHandle;
    AudioParams audioParams;
    EffectChoices effectChoice;
    RtUserData userData;
    
    const char* DEVICE_NAME = "plughw:0,0";

    // setup PCM device
    snd_pcm_uframes_t period = FRAMES_PER_BUFFER;
    snd_pcm_uframes_t buffer = FRAMES_PER_BUFFER * 4;

    setupPCM(DEVICE_NAME, &inHandle, SND_PCM_STREAM_CAPTURE, 1, 44100, period, buffer);
    setupPCM(DEVICE_NAME, &outHandle, SND_PCM_STREAM_PLAYBACK, 2, 44100, period, buffer);
    snd_pcm_link(inHandle, outHandle);  // makes sure playback clock starts when record starts
  
    initData(userData, audioParams, effectChoice);

    // Main loop: show menu, stream, then return to menu until user exits
    while (true) {
        bool keepRunning = menuFunction(effectChoice);
        if (!keepRunning) break;	

        // wait until user stops this session; then return to menu
        printf("Streaming... Press ENTER to stop and return to menu\n");

        bool streaming = true;
        int writePtr = 0;
        int readPtr = 0;

        std::vector<SAMPLE> inputBlock(FRAMES_PER_BUFFER);
        std::vector<SAMPLE> outputBlock(FRAMES_PER_BUFFER * 2);

        while (streaming){
	    snd_pcm_sframes_t framesRead = snd_pcm_readi(inHandle, inputBlock.data(), FRAMES_PER_BUFFER);
	    
	    if (framesRead == -EPIPE) {     // xrun 
	    	snd_pcm_prepare(inHandle);
		    continue;
	    }
        else if (framesRead < 0){
            // fatal error, exit
            break;
        }

	    // *** process ***
		processBlock(
			inputBlock.data(),
			outputBlock.data(),
			framesRead,
			&userData
			);

		// write to output
		snd_pcm_sframes_t framesWritten = snd_pcm_writei(outHandle, outputBlock.data(), framesRead);
		if (framesWritten == -EPIPE)    // xrun
		    snd_pcm_prepare(outHandle);
            continue;
	    }
        else if (framesRead < 0)
            break;

	    // check for ENTER
	    struct pollfd pfds;
	    pfds.fd = STDIN_FILENO;
	    pfds.events = POLLIN;
	    if (poll(&pfds, 1, 0) > 0){
	    	char c;
            read(STDIN_FILENO, &c, 1);
            if (c == '\n'){
                streaming = false;
                resetData(userData);
            }
        }

        snd_pcm_drain(inHandle);
        snd_pcm_drain(outHandle);

        // reset effect flags so menu starts clean next time
        effectChoice = EffectChoices();
    }
}
