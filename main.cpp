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
#include <string>
#include <ctime>

#include "menu.h"
#include "realtime_callback.h"
#include "discrete_callback.h"
#include "types.h"

using namespace std;

// Global Variables
EffectChoices effectChoice;
AudioParams audioParams;

// User Definitions
#define FRAMES_PER_BUFFER 64
#define NUM_SECONDS     5
#define PA_SAMPLE_TYPE  paFloat32
#define PRINTF_S_FORMAT "%.8f"
#define CONTINUOUS 1

// prototype
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
    
    // Declare Stream Parameters
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream *inStream, *outStream;

    // Error Checking
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio initialization failed: %s\n", Pa_GetErrorText(err));
    return -1;
    }
    
    paTestData data;
    int totalFrames;
    int numSamples;
    int numBytes;
    SAMPLE max, val;
    double average;

    //print i/o info
    const PaDeviceInfo *inputInfo = Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
    const PaDeviceInfo *outputInfo = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());

    printf("Input device: %s\n", inputInfo->name);
    printf("Channel count: %d\n", inputInfo->maxInputChannels);
    printf("Input sample rate: %f\n", inputInfo->defaultSampleRate);
    printf("Output device: %s\n", outputInfo->name);
    printf("Channel count: %d\n", outputInfo->maxOutputChannels);
    printf("Output sample rate: %f\n", outputInfo->defaultSampleRate);

    // input
    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default input device.\n");
        goto done;
    }

    inputParameters.channelCount = audioParams.IN_CHANNELS;  // mono input
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency =
        Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    // output
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr, "Error: No default output device.\n");
        goto done;
    }
    outputParameters.channelCount = audioParams.OUT_CHANNELS;  // must match input
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency =
        Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    
    //memory allocation
    data.maxFrameIndex = totalFrames = NUM_SECONDS * audioParams.SAMPLE_RATE;
    data.frameIndex = 0;
    numSamples = totalFrames * audioParams.IN_CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data.recordedSamples = (SAMPLE *)malloc(sizeof(SAMPLE) * data.maxFrameIndex * audioParams.IN_CHANNELS);
    memset(data.recordedSamples, 0, sizeof(SAMPLE) * data.maxFrameIndex * audioParams.IN_CHANNELS);
    if( data.recordedSamples == NULL ){
        printf("Could not allocate record array.\n");
        goto done;
    }
    
    //empty data
    for(int i=0; i<numSamples; i++ ) {
        data.recordedSamples[i] = 0;
        data.recordedSamples[i] = 0;
    }
    if( err != paNoError ) goto done;

    //setting input parameters
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        goto done;
    }
    inputParameters.channelCount = audioParams.IN_CHANNELS; 
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    //record audio
    if (CONTINUOUS){
        // TODO: connect continuous recording
        int delayMs = AudioParams::DELAY_MS;
        int sampleRate = AudioParams::SAMPLE_RATE;
        int delayBufferSize = (sampleRate * delayMs) / 1000; // frames

        int pow2 = 1;
        while (pow2 < delayBufferSize) pow2 <<= 1;
        delayBufferSize = pow2;

        SAMPLE *delayBuffer = (SAMPLE*) malloc(sizeof(SAMPLE) * delayBufferSize);
        if (!delayBuffer) {
            fprintf(stderr, "Could not allocate delay buffer.\n");
            goto done;
        }
        memset(delayBuffer, 0, sizeof(SAMPLE) * delayBufferSize);

        RtUserData userData;
        userData.params = &audioParams;
        userData.effects = &effectChoice;
        userData.delayBuffer = delayBuffer;
        userData.delayBufferSize = delayBufferSize;
        userData.delayIndex = 0;
        userData.tremIncrement = 2.0 * audioParams.PI * audioParams.TREM_FREQ / (double)sampleRate;

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
        err = Pa_StopStream(inStream);
        if (err != paNoError) goto done;
        printf("Stopped streaming.\n");
        Pa_CloseStream(inStream);
        return 0;
    }

    else{
        err = Pa_OpenDefaultStream(
            &inStream,
            audioParams.IN_CHANNELS,
            0,
            PA_SAMPLE_TYPE,
            audioParams.SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            recordCallback,
            &data);

        if(err != paNoError) goto done;
        err = Pa_StartStream(inStream);
        if(err != paNoError) goto done;
        printf("\nRecording... \n"); fflush(stdout);

        while((err = Pa_IsStreamActive(inStream)) == 1)
        {
            Pa_Sleep(1000);
            printf("index = %d\n", data.frameIndex); fflush(stdout);
        }
        if(err < 0) goto done;
        
        //end recording process
        Pa_StopStream(inStream);
        Pa_CloseStream(inStream);
    
        //amplitude
        max = 0;
        average = 0.0;
        for(int i=0; i<numSamples; i++)
        {
            val = data.recordedSamples[i];
            if(val < 0) val = -val;
            if(val > max)
            {
                max = val;
            }
            average += val;
        }

        average = average / (double)numSamples;

        printf("sample max amplitude = " PRINTF_S_FORMAT "\n", max );
        printf("sample average = %lf\n", average );
    }

    data.frameIndex = 0;
    
    if (outputParameters.device == paNoDevice){
        fprintf(stderr, "Error: No default output device.\n");
        goto done;
    }
    outputParameters.channelCount = audioParams.OUT_CHANNELS;
    outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    
    //playback
    printf("\nNow playing back...\n"); fflush(stdout);
    err = Pa_OpenDefaultStream(
            &outStream,
            0,
            audioParams.OUT_CHANNELS,
            PA_SAMPLE_TYPE,
            audioParams.SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            playCallback,
            &data );
    if( err != paNoError ) goto done;

    if(outStream)
    {
        err = Pa_StartStream(outStream);
        if(err != paNoError) goto done;
        
        printf("Waiting for playback to finish.\n"); fflush(stdout);

        while( (err = Pa_IsStreamActive(outStream)) == 1) Pa_Sleep(100);
        if(err < 0) goto done;
        
        err = Pa_CloseStream(outStream);
        if(err != paNoError) goto done;
        
        printf("Done.\n"); fflush(stdout);
    }
    //end playback process
    Pa_StopStream(outStream);
    Pa_CloseStream(outStream);
    
 done:   
    //CLOSE PROGRAM

    
    Pa_Terminate();

    //free memory in case it wasn't freed 
    if (data.recordedSamples != NULL) {
        free(data.recordedSamples);
        data.recordedSamples = NULL;    //safety measure
    }

    //Error handling
    if (err != paNoError) {
        fprintf(stderr, "An error occurred while using the PortAudio stream\n");
        fprintf(stderr, "Error number: %d\n", err);
        fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
        err = 1; 
    }
    
    return err;
}
