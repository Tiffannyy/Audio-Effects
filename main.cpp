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
#define FRAMES_PER_BUFFER 64
#define CIRCULAR_MULTIPLIER 2.5
#define CONTINUOUS 1

//main function
int main(){

    // Declare stream parameters
    int err;
    snd_pcm_t *inHandle;
    snd_pcm_t *outHandle;
    snd_pcm_hw_params_t *params;

    // open PCM for input and output
    // NOTE: can change default name to specific device here
    const char* DEVICE_NAME = "hw:0,0";

    err = snd_pcm_open(&inHandle, DEVICE_NAME, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
    if (err < 0) {
        fprintf(stderr, "Error opening input PCM device: %s\n", snd_strerror(err));
        return err;
    }
    err = snd_pcm_open(&outHandle, DEVICE_NAME, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (err < 0) {
        fprintf(stderr, "Error opening output PCM device: %s\n", snd_strerror(err));
        snd_pcm_close(inHandle);
        return err;
    }

    // input parameters
    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(inHandle, params);
    snd_pcm_hw_params_set_access(inHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(inHandle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(inHandle, params, AudioParams::CHANNELS);
    snd_pcm_hw_params_set_rate(inHandle, params, AudioParams::SAMPLE_RATE, 0);
    err = snd_pcm_hw_params(inHandle, params);
    if (err < 0) {
        fprintf(stderr, "Error setting input PCM parameters: %s\n", snd_strerror(err));
        snd_pcm_close(inHandle);
        snd_pcm_close(outHandle);
        return err;
    }
    snd_pcm_prepare(inHandle);

    // output parameters
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(outHandle, params);
    snd_pcm_hw_params_set_access(outHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(outHandle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(outHandle, params, AudioParams::CHANNELS);
    snd_pcm_hw_params_set_rate(outHandle, params, AudioParams::SAMPLE_RATE, 0);
    err = snd_pcm_hw_params(outHandle, params);
    if (err < 0) {
        fprintf(stderr, "Error setting output PCM parameters: %s\n", snd_strerror(err));
        snd_pcm_close(inHandle);
        snd_pcm_close(outHandle);
        return err;
    }
    snd_pcm_prepare(outHandle);


    // setup user data
    AudioParams audioParams;
    EffectChoices effectChoice;
    RtUserData userData;
    userData.params = &audioParams;
    userData.effects = &effectChoice;

    int delayMs = AudioParams::DELAY_MS;
    int sampleRate = AudioParams::SAMPLE_RATE;

    userData.tremIncrement = 2.0 * audioParams.PI * audioParams.TREM_FREQ / (double)sampleRate;
    userData.delaySize = max(1, delayMs * sampleRate / 1000);
    userData.delayBuffer.assign(userData.delaySize, 0.0f);
    userData.reverbSize = AudioParams::SAMPLE_RATE;
    userData.reverbBuffer.assign(userData.reverbSize, 0.0f);

    float tapsMs[AudioParams::REVERB_TAPS] = {40.0f, 50.0f, 60.0f, 80.0f, 110.0f};
    for (int i = 0; i < AudioParams::REVERB_TAPS; ++i){
    	userData.reverbDelay[i] = tapsMs[i] * sampleRate / 1000;
	userData.reverbGain[i] = expf(-tapsMs[i] / audioParams.reverbDecay);
	userData.reverbIndex[i] = 0;
    }

    assert(ud->delayBuffer.size() == ud->delaySize);

    // Main loop: show menu, stream, then return to menu until user exits
    while (true) {
        bool keepRunning = menuFunction(effectChoice);
        if (!keepRunning) break; // user chose to exit
//        userData.reverbGain[0] = 0.4f;
//        userData.reverbGain[1] = 0.3f;
//        userData.reverbGain[2] = 0.3f;
//        userData.reverbGain[3] = 0.3f;
//        userData.reverbGain[4] = 0.2f;
	userData.delayIndex = 0;

        // wait until user stops this session; then return to menu
        printf("Streaming... Press ENTER to stop and return to menu\n");

        bool streaming = true;

	const int CIRCULAR_SIZE = FRAMES_PER_BUFFER * CIRCULAR_MULTIPLIER;
	std::vector<SAMPLE> circBuffer(CIRCULAR_SIZE);
	int writePtr = 0;
	int readPtr = 0;

	std::vector<SAMPLE> inputBlock(FRAMES_PER_BUFFER * AudioParams::CHANNELS);
	std::vector<SAMPLE> outputBlock(FRAMES_PER_BUFFER * AudioParams::CHANNELS);

        while (streaming){
	    snd_pcm_sframes_t framesRead = snd_pcm_readi(inHandle, inputBlock.data(), FRAMES_PER_BUFFER);
	    
	    if (framesRead < 0) {
	    	snd_pcm_prepare(inHandle);
		continue;
	    }

	    // copy into circBuffer
	    for (int i = 0; i < framesRead * AudioParams::CHANNELS; ++i){
	    	circBuffer[writePtr] = inputBlock[i];
		writePtr = (writePtr + 1) % CIRCULAR_SIZE;
	    }

	    // *** process ***
	    int avail = (writePtr - readPtr + CIRCULAR_SIZE) % CIRCULAR_SIZE;
	    while (avail >= FRAMES_PER_BUFFER * AudioParams::CHANNELS){
	    	for (int i = 0; i < FRAMES_PER_BUFFER * AudioParams::CHANNELS; ++i){
		    inputBlock[i] = circBuffer[readPtr];
		    readPtr = (readPtr + 1) % CIRCULAR_SIZE;
		}

		// apply effects
		processBlock(
			inputBlock.data(),
			outputBlock.data(),
			FRAMES_PER_BUFFER,
			&userData
			);

		// write to output
		snd_pcm_sframes_t framesWritten = snd_pcm_writei(outHandle, outputBlock.data(), FRAMES_PER_BUFFER);
		if (framesWritten < 0)
		    snd_pcm_prepare(outHandle);

		avail -= FRAMES_PER_BUFFER * AudioParams::CHANNELS;
	    }

	    // check for ENTER
	    struct pollfd pfds;
	    pfds.fd = STDIN_FILENO;
	    pfds.events = POLLIN;
	    if (poll(&pfds, 1, 0) > 0){
	    	char c;
		read(STDIN_FILENO, &c, 1);
		if (c == '\n')
		    streaming = false;
	    }
        }

        snd_pcm_drain(inHandle);
        snd_pcm_drain(outHandle);

        // reset effect flags so menu starts clean next time
        effectChoice = EffectChoices();
    }
}
