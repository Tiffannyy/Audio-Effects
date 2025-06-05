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
#include <cmath>
#include <string>
#include <ctime>

#include "menu.h"
#include "non_continous_call.h"
#include "types.h"

using namespace std;

// Global Variables
EffectChoices effectChoice = {false, false, false};

// User Definitions
#define FRAMES_PER_BUFFER 1024
#define NUM_SECONDS     5
#define WRITE_TO_FILE   0
#define PA_SAMPLE_TYPE  paFloat32
#define PRINTF_S_FORMAT "%.8f"
#define CONTINUOUS 0


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
    //input
    inputParameters.device = Pa_GetDefaultInputDevice();  //default input device
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo(inputParameters.device); 
    if (deviceInfo) {
    printf("Input device: %s\n", deviceInfo->name);
    printf("Maximum input channels: %d\n", deviceInfo->maxInputChannels);
    printf("Default input latency: %f seconds\n", deviceInfo->defaultLowInputLatency);
    printf("Sample rate: %f\n", deviceInfo->defaultSampleRate);
    }
    
    //output
    outputParameters.device = Pa_GetDefaultOutputDevice(); //default output device
    const PaDeviceInfo *OutdeviceInfo = Pa_GetDeviceInfo(outputParameters.device); 
    if (OutdeviceInfo){
    printf("Output device: %s\n", OutdeviceInfo->name);
    printf("Default output latency: %f seconds\n", OutdeviceInfo->defaultLowOutputLatency);
    }
    
    //memory allocation
    data.maxFrameIndex = totalFrames = NUM_SECONDS * audioParams.SAMPLE_RATE;
    data.frameIndex = 0;
    numSamples = totalFrames * audioParams.NUM_CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data.recordedSamples = (SAMPLE *)malloc(sizeof(SAMPLE) * data.maxFrameIndex * audioParams.NUM_CHANNELS);
    memset(data.recordedSamples, 0, sizeof(SAMPLE) * data.maxFrameIndex * audioParams.NUM_CHANNELS);   
    memset(delayBuffer, 0, sizeof(SAMPLE) * DELAY_BUFFER_SIZE);
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
    inputParameters.channelCount = audioParams.NUM_CHANNELS; 
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    //record audio
    if (CONTINUOUS){
    
    }
    else{
        err = Pa_OpenDefaultStream(
                &inStream,
                audioParams.NUM_CHANNELS,
                0,
                PA_SAMPLE_TYPE,
                audioParams.SAMPLE_RATE,
                FRAMES_PER_BUFFER,
                recordCallback,
                &data );
    }
    if( err != paNoError ) goto done;

    err = Pa_StartStream( inStream );
    if( err != paNoError ) goto done;
    printf("\nRecording... \n"); fflush(stdout);

    while( ( err = Pa_IsStreamActive(inStream )) == 1 )
    {
        Pa_Sleep(1000);
        printf("index = %d\n", data.frameIndex ); fflush(stdout);
    }
    if( err < 0 ) goto done;
    
    //end recording process
    Pa_StopStream(inStream);
    Pa_CloseStream(inStream);
 
    //amplitude
    max = 0;
    average = 0.0;
    for(int i=0; i<numSamples; i++ )
    {
        val = data.recordedSamples[i];
        if( val < 0 ) val = -val;
        if( val > max )
        {
            max = val;
        }
        average += val;
    }

    average = average / (double)numSamples;

    printf("sample max amplitude = " PRINTF_S_FORMAT "\n", max );
    printf("sample average = %lf\n", average );

    /* Write recorded data to a file. */
#if WRITE_TO_FILE
    {
        FILE  *fid;
        fid = fopen("recorded.raw", "wb");
        if( fid == NULL )
        {
            printf("Could not open file.");
        }
        else
        {
            fwrite( data.recordedSamples, NUM_CHANNELS * sizeof(SAMPLE), totalFrames, fid );
            fclose( fid );
            printf("Wrote data to 'recorded.raw'\n");
        }
    }
#endif

    data.frameIndex = 0;
    
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
        goto done;
    }
    outputParameters.channelCount = 2;
    outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    
    //playback
    printf("\nNow playing back...\n"); fflush(stdout);
    err = Pa_OpenDefaultStream(
            &outStream,
            0,
            audioParams.NUM_CHANNELS,
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
