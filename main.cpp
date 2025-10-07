/*
 * main.cpp
 * DSP Program
 * 
 * Tiffany Liu
 * 
 * Description: Main file for DSP program. Point of access.
*/

//Libraries
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <portaudio.h>
#include <cstring>
#include <ctime>

#include "menu.h"
#include "realtime_callback.h"
#include "types.h"

using namespace std;

// Global Variables
EffectChoices effectChoice;
AudioParams audioParams;

// User Definitions
#define FRAMES_PER_BUFFER 16
#define PA_SAMPLE_TYPE  paFloat32
#define PRINTF_S_FORMAT "%.8f"
#define CONTINUOUS 1                // 0 for 5s sample, 1 for continuous
#define DEBUG 1                     // 1 to print debug info

SAMPLE *delayBuffer = nullptr;
int delayIndex = 0;

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

    // Menu
    menuFunction(effectChoice);
    
    // Declare stream parameters
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream *inStream, *outStream;

    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio initialization failed: %s\n", Pa_GetErrorText(err));
    return -1;
    }

    //print i/o info if DEBUG 1
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
        Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
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
        Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    // setup user data
    RtUserData userData;
    userData.params = &audioParams;
    userData.effects = &effectChoice;

    // record audio
    if (CONTINUOUS){
        // TODO: connect continuous recording

        int delayMs = AudioParams::DELAY_MS;
        int sampleRate = AudioParams::SAMPLE_RATE;
        int delayBufferSize = (sampleRate * delayMs) / 1000; // frames

        // remaining user data
        userData.delayBufferSize = delayBufferSize;
        userData.tremIncrement = 2.0 * audioParams.PI * audioParams.TREM_FREQ / (double)sampleRate;

        int pow2 = 1;
        while (pow2 < delayBufferSize) pow2 <<= 1;
        delayBufferSize = pow2;

        // allocate delay buffer
        delayBuffer = (SAMPLE*) malloc(sizeof(SAMPLE) * delayBufferSize);
        if (!delayBuffer) {
            fprintf(stderr, "Could not allocate delay buffer.\n");
            goto done;
        }
        memset(delayBuffer, 0, sizeof(SAMPLE) * delayBufferSize);

        // set delayBuffer and delayIndex
        userData.delayBuffer = delayBuffer;
        userData.delayIndex = delayIndex;

        // open stream
        err =  Pa_OpenStream(&inStream,
                             &inputParameters,
                             &outputParameters,
                             audioParams.SAMPLE_RATE,
                             FRAMES_PER_BUFFER,
                             0,
                             streamCallback,
                             &userData);

        if (err != paNoError) goto done;

        err = Pa_StartStream(inStream);
        if (err != paNoError) goto done;
        
        // wait until user exits
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
        printf("Streaming... Press ENTER to stop program\n");
        getchar();
        Pa_StopStream(inStream);
        Pa_CloseStream(inStream);
        return 0;
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
