// /* 
//  * discrete_callback.h
//  * DSP Program
//  * 
//  * Tiffany Liu
//  * 4 June 2025
//  * 
//  * Description: Contains the input and output callback functions for discrete processing. 
//  * 
// */

// // Libraries
// #ifndef DISCRETE_CALLBACK_H
// #define DISCRETE_CALLBACK_H

// #include <portaudio.h>
// #include <cstdlib>
// #include <cmath>

// #include "types.h"

// using namespace std;

// // Global Variables
// extern AudioParams audioParams;

// // User Definitions
// #define SAMPLE_SILENCE  0.0f
// #define DELAY_BUFFER_SIZE ((audioParams.SAMPLE_RATE * audioParams.DELAY_MS) / 1000)

// extern SAMPLE delayBuffer[DELAY_BUFFER_SIZE];
// extern int delayIndex;

// // Record callback
// static int recordCallback(const void *inputBuffer, void *outputBuffer,
//                           unsigned long framesPerBuffer,
//                           const PaStreamCallbackTimeInfo *timeInfo,
//                           PaStreamCallbackFlags statusFlags,
//                           void *userData)
// {
//     paTestData *data = (paTestData *)userData;
//     const SAMPLE *rptr = (const SAMPLE *)inputBuffer;
//     SAMPLE *wptr = &data->recordedSamples[data->frameIndex * audioParams.IN_CHANNELS];
//     unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;
//     int finished = (framesLeft < framesPerBuffer) ? paComplete : paContinue;
//     unsigned long framesToCalc = (framesLeft < framesPerBuffer) ? framesLeft : framesPerBuffer;

//     (void)outputBuffer;  // Prevent unused variable warning
//     (void)timeInfo;
//     (void)statusFlags;
//     (void)userData;

//     if (inputBuffer == NULL) {
//         for (int i = 0; i < framesToCalc; i++) {
//             *wptr++ = SAMPLE_SILENCE;       // No audio input, fill with silence
//         }
//     }
//     else {
//         for (int i = 0; i < framesToCalc; i++) {
//             *wptr++ = *rptr++;              // Copy audio data from input buffer to recorded samples
//         }
//     }
//     data->frameIndex += framesToCalc;
//     return finished;
// }

// // Playback
// static int playCallback( const void *inputBuffer, void *outputBuffer,
//                          unsigned long framesPerBuffer,
//                          const PaStreamCallbackTimeInfo* timeInfo,
//                          PaStreamCallbackFlags statusFlags,
//                          void *userData){
//     paTestData *data = (paTestData*)userData;
//     SAMPLE *rptr = &data->recordedSamples[data->frameIndex * audioParams.OUT_CHANNELS];
//     SAMPLE *wptr = (SAMPLE*)outputBuffer;
//     unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;
//     unsigned long framesToPlay = (framesLeft < framesPerBuffer) ? framesLeft : framesPerBuffer;

//     (void) inputBuffer; // Unused variables
//     (void) timeInfo;
//     (void) statusFlags;
//     (void) userData;
    
//     // For tremolo convolution
//     int waveLen = audioParams.SAMPLE_RATE / audioParams.TREM_FREQ;
//     SAMPLE sineWave[waveLen];
//     for (int i = 0; i < waveLen; i++)
//         sineWave[i] = sin(2.0 * audioParams.PI * i / waveLen);
    
//     // Delay
//     SAMPLE drySignal, wetSignal, outputSample;
        
//     // Playback        
//     for (unsigned long i = 0; i < framesToPlay; i++) {
//         // Normal
//         if (effectChoice.norm)
//             *wptr++ = *rptr++;
//         // Tremolo
//         else if (effectChoice.trem){
//             *wptr++ = *rptr++ * sineWave[((int)(audioParams.tremPhase) % waveLen)];
//             audioParams.tremPhase += 1.0;
//             if (audioParams.tremPhase >= waveLen) {
//                 audioParams.tremPhase = 0.0; // reset phase after wavelength
//             }
//         }
//         // Delay
//         else if (effectChoice.delay) {
//             drySignal = *rptr++;
//             wetSignal = delayBuffer[delayIndex] + drySignal;            //get rid of + drySignal to hear only delayed part.
//             outputSample = (audioParams.MIX * wetSignal);                           //relating to above comment, add + (1 - MIX) * drySignal to this line
//             delayBuffer[delayIndex] = drySignal + (audioParams.FEEDBACK * wetSignal);
//             delayIndex = (delayIndex + 1) % DELAY_BUFFER_SIZE;
//             *wptr++ = outputSample;
//         }
//         // Others
//         if (framesLeft < framesPerBuffer) {
//             for (unsigned long i = framesLeft; i < framesPerBuffer; i++) {
//                 *wptr++ = SAMPLE_SILENCE;  //Fill remaining buffer with silence
//             }
//         return paComplete;
//         }
//     }
//     data->frameIndex += framesToPlay;
//     return paContinue;
// }

// #endif // DISCRETE_CALLBACK_H