/*
 * Audio Effects Program
 * Tiffany Liu
 * 
 * IMPORTANT: for mac, because of permission issues, compile and run in terminal.
 *            Commands:
 *            g++ -std=c++11 -o recordAudio main.cpp -lportaudio
 *            ./recordAudio 
 */

//Libraries
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <portaudio.h>
#include <cmath>
#include <string>
using namespace std;

//user definitions
#define TREM_FREQ 2.0 //tremolo frequency (Hz). lower the freq, the slower the tremolo effect vice versa
#define TREM_DEPTH 0.5 // tremolo depth. 0 has no effect, 1 has full effect
#define SAMPLE_RATE  48000 //44100 or 48000  change to sample rate listed in output
#define FRAMES_PER_BUFFER 1024
#define NUM_SECONDS     5
#define NUM_CHANNELS    1
#define WRITE_TO_FILE   0
#define PA_SAMPLE_TYPE  paFloat32
#define PRINTF_S_FORMAT "%.8f"
#define SAMPLE_SILENCE  0.0f
#define DELAY_MS 1000          // Delay in milliseconds
#define FEEDBACK 0        // Feedback amount (0 to 1)
#define MIX 0.5            // Mix between original and delayed signals

typedef float SAMPLE;
typedef struct{
    int          frameIndex;
    int          maxFrameIndex;
    SAMPLE      *recordedSamples;
} paTestData;

//global variables

const double PI = 3.14159265358979323846;
double tremPhase = 0.0;
bool trem = 0;
bool norm = 0;
bool delay = 0;

// Circular buffer for delay effect
#define DELAY_BUFFER_SIZE ((SAMPLE_RATE * DELAY_MS) / 1000)
SAMPLE delayBuffer[DELAY_BUFFER_SIZE];
int delayIndex = 0;

//menu function
bool menuFunction(){
    bool validChoice = 0;
    char userChoice;
    cout << "*------ Audio Effects Program ------*\n" << "Effects Options:\n";
    cout << "(0) Exit Program" << endl; 
    cout << "(1) No Effects" << endl;
    cout << "(2) Tremolo" << endl;
    cout << "(3) Delay" << endl;                  
//    cout << "(4) Volume Adjuster" << endl;
    cout << "Enter the integer value of the effect you would like to apply: ";
    cin >> userChoice;
    
    switch (userChoice){
            case '0':
                cout << "Ending program..." << endl;
                exit(0);
                break;
            case '1':                       //no effect
                norm = 1;
                validChoice = 1;
                break;
            case '2':                       //tremolo
                trem = 1;
                validChoice = 1;
                break;
            case '3':
                delay = 1;
                validChoice = 1;
                break;
            default:
                break;
        }
    while (!validChoice){
    cout << "Enter a valid option: ";
    cin >> userChoice; 
        switch (userChoice){
            case '0':
                cout << "Ending program..." << endl;
                exit(0);
                break;
            case '1':                       //no effect
                norm = 1;
                validChoice = 1;
                break;
            case '2':                       //tremolo
                trem = 1;
                validChoice = 1;
                break;
            case '3':
                delay = 1;
                validChoice = 1;
                break;
            default:
                break;
        }
      
    }
    return true;
}
//record function
static int recordCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData)
{
    paTestData *data = (paTestData *)userData;
    const SAMPLE *rptr = (const SAMPLE *)inputBuffer;
    SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;
    int finished = (framesLeft < framesPerBuffer) ? paComplete : paContinue;
    unsigned long framesToCalc = (framesLeft < framesPerBuffer) ? framesLeft : framesPerBuffer;

    (void)outputBuffer;  //Prevent unused variable warning
    (void)timeInfo;
    (void)statusFlags;
    (void)userData;

    if (inputBuffer == NULL) {
        for (int i = 0; i < framesToCalc; i++) {
            *wptr++ = SAMPLE_SILENCE;       // No audio input, fill with silence
        }
    }
    else {
        for (int i = 0; i < framesToCalc; i++) {
            *wptr++ = *rptr++;              // Copy audio data from input buffer to recorded samples
        }
    }
    data->frameIndex += framesToCalc;
    return finished;
}

//playback function
static int playCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData){
    paTestData *data = (paTestData*)userData;
    SAMPLE *rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    SAMPLE *wptr = (SAMPLE*)outputBuffer;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;
    unsigned long framesToPlay = (framesLeft < framesPerBuffer) ? framesLeft : framesPerBuffer;

    (void) inputBuffer; //unused variables
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;
    
    //for tremolo convolution
    int waveLen = SAMPLE_RATE / TREM_FREQ;
    SAMPLE sineWave[waveLen];
    for (int i = 0; i < waveLen; i++)
        sineWave[i] = sin(2.0 * PI * i / waveLen);
    
    //delay
    SAMPLE drySignal, wetSignal, outputSample;
        
    //playback        
    for (unsigned long i = 0; i < framesToPlay; i++) {
        //normal
        if (norm)
            *wptr++ = *rptr++;
        //tremolo
        else if (trem){
            *wptr++ = *rptr++ * sineWave[((int)(tremPhase) % waveLen)];
            tremPhase += 1.0;
            if (tremPhase >= waveLen) {
                tremPhase = 0.0; //reset phase after wavelength
            }
        }
        //delay
        else if (delay) {
            drySignal = *rptr++;
            wetSignal = delayBuffer[delayIndex] + drySignal;            //get rid of + drySignal to hear only delayed part.
            outputSample = (MIX * wetSignal);                           //relating to above comment, add + (1 - MIX) * drySignal to this line
            delayBuffer[delayIndex] = drySignal + (FEEDBACK * wetSignal);
            delayIndex = (delayIndex + 1) % DELAY_BUFFER_SIZE;
            *wptr++ = outputSample;
        }
        //others
        if (framesLeft < framesPerBuffer) {
            for (unsigned long i = framesLeft; i < framesPerBuffer; i++) {
                *wptr++ = SAMPLE_SILENCE;  //Fill remaining buffer with silence
            }
        return paComplete;
        }
    }
    data->frameIndex += framesToPlay;
    return paContinue;
}


//main function
int main() {
    menuFunction();
    
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream *inStream, *outStream;
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
    data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE;
    data.frameIndex = 0;
    numSamples = totalFrames * NUM_CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data.recordedSamples = (SAMPLE *)malloc(sizeof(SAMPLE) * data.maxFrameIndex * NUM_CHANNELS);
    memset(data.recordedSamples, 0, sizeof(SAMPLE) * data.maxFrameIndex * NUM_CHANNELS);   
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
    inputParameters.channelCount = NUM_CHANNELS; 
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    //record audio
    err = Pa_OpenDefaultStream(
            &inStream,
            NUM_CHANNELS,
            0,
            PA_SAMPLE_TYPE,
            SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            recordCallback,
            &data );
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
            NUM_CHANNELS,
            PA_SAMPLE_TYPE,
            SAMPLE_RATE,
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
