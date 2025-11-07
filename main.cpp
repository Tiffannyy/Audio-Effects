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
#include <portaudio.h>
#include <limits>
#include <string>
#include <iostream>

#include "menu.h"
#include "realtime_callback.h"
#include "types.h"

using namespace std;

// Global Variables
EffectChoices effectChoice;
AudioParams audioParams;
RtUserData userData;

// User Definitions
#define FRAMES_PER_BUFFER 128
#define PA_SAMPLE_TYPE  paFloat32
#define PRINTF_S_FORMAT "%.8f"
#define CONTINUOUS 1                // 0 for 5s sample, 1 for continuous
#define DEBUG 1                     // 1 to print debug info

// streamCallback prototype
static int streamCallback(
    const void *input, 
    void *output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData
);

//main function
int main() {

    // Declare stream parameters
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream *inStream = NULL;

    // Initialize PortAudio once
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio initialization failed: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    // print i/o info if DEBUG
    #ifdef DEBUG
        const PaDeviceInfo *inputInfo = Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
        const PaDeviceInfo *outputInfo = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());

        printf("Input device: %s\n", inputInfo->name);
        printf("Channel count: %d\n", inputInfo->maxInputChannels);
        printf("Input sample rate: %f\n", inputInfo->defaultSampleRate);
        printf("Output device: %s\n", outputInfo->name);
        printf("Channel count: %d\n", outputInfo->maxOutputChannels);
        printf("Output sample rate: %f\n", outputInfo->defaultSampleRate);
    #endif

    // input parameters
    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default input device.\n");
        goto done;
    }
    inputParameters.channelCount = audioParams.IN_CHANNELS;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency =
        Pa_GetDeviceInfo(inputParameters.device)->defaultHighInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    // output parameters
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default output device.\n");
        goto done;
    }
    outputParameters.channelCount = audioParams.OUT_CHANNELS;
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency =
        Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

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
            userData.reverbGain[i] = expf(-tapsMs[i] / AudioParams::reverbDecay);
            userData.reverbIndex[i] = 0;
        }

        userData.reverbGain[0] = 0.4f;
        userData.reverbGain[1] = 0.3f;
        userData.reverbGain[2] = 0.3f;
        userData.reverbGain[3] = 0.3f;
        userData.reverbGain[4] = 0.2f;

        // open stream
        err =  Pa_OpenStream(&inStream,
                             &inputParameters,
                             &outputParameters,
                             sampleRate,
                             FRAMES_PER_BUFFER,
                             0,
                             streamCallback,
                             &userData);

        if (err != paNoError) goto done;

        err = Pa_StartStream(inStream);
        if (err != paNoError) goto done;

        // wait until user stops this session; then return to menu
        printf("Streaming... Press ENTER to stop and return to menu\n");
        // Clear any leftover input from previous std::cin >> calls
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::string line;
        std::getline(std::cin, line);

        Pa_StopStream(inStream);
        Pa_CloseStream(inStream);

        // reset effect flags so menu starts clean next time
        effectChoice = EffectChoices();
        // clear buffers to free memory if desired
        userData.delayBuffer.clear();
        userData.reverbBuffer.clear();
        // loop back to menu
    }

    // Clean up and exit program
 done:
    Pa_Terminate();

    // Error handling
    if (err != paNoError) {
        fprintf(stderr, "An error occurred while using the PortAudio stream\n");
        fprintf(stderr, "Error number: %d\n", err);
        fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
        err = 1; 
    }
    
    return err;
}
