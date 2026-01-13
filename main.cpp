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
// #include <portaudio.h>
#include <alsa/asoundlib.h>
#include <limits>
#include <string>
#include <iostream>
#include <poll.h>
#include <unistd.h>

#include "menu.h"
#include "realtime_callback.h"
#include "types.h"

using namespace std;

// Global Variables
EffectChoices effectChoice;
AudioParams audioParams;
RtUserData userData;

// User Definitions
#define FRAMES_PER_BUFFER 256
#define PRINTF_S_FORMAT "%.8f"
#define CONTINUOUS 1                // 0 for 5s sample, 1 for continuous

//main function
int main() {

    // Declare stream parameters
    int err;
    snd_pcm_t *inHandle;
    snd_pcm_t *outHandle;

    snd_pcm_hw_params_t *params;

    // open PCM for input and output
    // NOTE: can change default name to specific device here
    const char* DEVICE_NAME = "hw:0,0";

    err = snd_pcm_open(&inHandle, DEVICE_NAME, SND_PCM_STREAM_CAPTURE, 0);
    if (err < 0) {
        fprintf(stderr, "Error opening input PCM device: %s\n", snd_strerror(err));
        return err;
    }
    err = snd_pcm_open(&outHandle, DEVICE_NAME, SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        fprintf(stderr, "Error opening output PCM device: %s\n", snd_strerror(err));
        snd_pcm_close(inHandle);
        return err;
    }

    snd_pcm_uframes_t period = FRAMES_PER_BUFFER;
    snd_pcm_uframes_t buffer = FRAMES_PER_BUFFER * 4;

    // input parameters
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(inHandle, params);
    snd_pcm_hw_params_set_access(inHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(inHandle, params, SND_PCM_FORMAT_FLOAT_LE);
    snd_pcm_hw_params_set_channels(inHandle, params, AudioParams::IN_CHANNELS);
    snd_pcm_hw_params_set_rate(inHandle, params, audioParams.SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_period_size_near(inHandle, params, &period, 0);
    snd_pcm_hw_params_set_buffer_size_near(inHandle, params, &buffer);

    err = snd_pcm_hw_params(inHandle, params);
    if (err < 0) {
        fprintf(stderr, "Error setting input PCM parameters: %s\n", snd_strerror(err));
        snd_pcm_close(inHandle);
        snd_pcm_close(outHandle);
        return err;
    }

    // output parameters
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(outHandle, params);
    snd_pcm_hw_params_set_access(outHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(outHandle, params, SND_PCM_FORMAT_FLOAT_LE);
    snd_pcm_hw_params_set_channels(outHandle, params, AudioParams::OUT_CHANNELS);
    snd_pcm_hw_params_set_rate(outHandle, params, audioParams.SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_period_size_near(outHandle, params, &period, 0);
    snd_pcm_hw_params_set_buffer_size_near(outHandle, params, &buffer);

    err = snd_pcm_hw_params(outHandle, params);
    if (err < 0) {
        fprintf(stderr, "Error setting output PCM parameters: %s\n", snd_strerror(err));
        snd_pcm_close(inHandle);
        snd_pcm_close(outHandle);
        return err;
    }

    // setup user data pointer
    userData.params = &audioParams;
    userData.effects = &effectChoice;

    // Main loop: show menu, stream, then return to menu until user exits
    while (true) {
        bool keepRunning = menuFunction(effectChoice);
        if (!keepRunning) break; // user chose to exit

        if (!CONTINUOUS) break;

        int delayMs = AudioParams::DELAY_MS;
        int sampleRate = AudioParams::SAMPLE_RATE;

        // remaining user data
        userData.tremIncrement = 2.0 * audioParams.PI * audioParams.TREM_FREQ / (double)sampleRate;

        // set delay and reverb params and reset buffers
        userData.delayIndex = 0;
        userData.delaySize = max(1, delayMs * sampleRate / 1000);
        userData.reverbSize = AudioParams::SAMPLE_RATE; // max 1 second delay buffer

        userData.delayBuffer.assign(userData.delaySize, 0.0f);
        userData.reverbBuffer.assign(userData.reverbSize, 0.0f);

        float tapsMs[AudioParams::REVERB_TAPS] = {40.0f, 50.0f, 60.0f, 80.0f, 110.0f};
        for (int i = 0; i < AudioParams::REVERB_TAPS; ++i){
            userData.reverbDelay[i] = tapsMs[i] * sampleRate / 1000;
            userData.reverbGain[i] = expf(-tapsMs[i] / audioParams.reverbDecay);
            userData.reverbIndex[i] = 0;
        }

        userData.reverbGain[0] = 0.4f;
        userData.reverbGain[1] = 0.3f;
        userData.reverbGain[2] = 0.3f;
        userData.reverbGain[3] = 0.3f;
        userData.reverbGain[4] = 0.2f;

        // wait until user stops this session; then return to menu
        printf("Streaming... Press ENTER to stop and return to menu\n");

        snd_pcm_prepare(inHandle);
        snd_pcm_prepare(outHandle);

        bool streaming = true;

        SAMPLE inputBuffer[FRAMES_PER_BUFFER * AudioParams::OUT_CHANNELS];
        SAMPLE outputBuffer[FRAMES_PER_BUFFER * AudioParams::OUT_CHANNELS];

        while (streaming){
            snd_pcm_sframes_t framesRead = snd_pcm_readi(inHandle, inputBuffer, FRAMES_PER_BUFFER);

            if (framesRead < 0){
                snd_pcm_prepare(inHandle);
                continue;
            }

            // call processing function, previously callback
            processBlock(
                inputBuffer,
                outputBuffer,
                framesRead,
                &userData
            );

            snd_pcm_sframes_t framesWritten = snd_pcm_writei(
                outHandle,
                outputBuffer,
                framesRead
            );

            if (framesWritten < 0){
                snd_pcm_prepare(outHandle);
                continue;
            }
            // check if user pressed ENTER to stop streaming
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
        // clear buffers to free memory if desired
        userData.delayBuffer.clear();
        userData.reverbBuffer.clear();
        // loop back to menu
    }

    snd_pcm_close(inHandle);
    snd_pcm_close(outHandle);
}
