/*
 * realtime_callback.h
 * DSP Program
 * 
 * Tiffany Liu
 * 5 June 2025
 * 
 * Description: Contains the callback function for realtime audio processing
 * 
*/

#ifndef REALTIME_CALLBACK_H
#define REALTIME_CALLBACK_H

using namespace std;

// User Definitions
#define SAMPLE_SILENCE  0.0f
#define DELAY_BUFFER_SIZE ((audioParams.SAMPLE_RATE * audioParams.DELAY_MS) / 1000)


// Callback Function
static int streamCallback(const void *inputBuffer, void *outputBuffer,
                     unsigned long framesPerBuffer,
                     const PaStreamCallbackTimeInfo* timeinfo,
                     PaStreamCallbackFlags statusFlags,
                     void *userData){
                
    SAMPLE *out = (SAMPLE*)outputBuffer;
    const SAMPLE *in = (const SAMPLE*)inputBuffer;
    RtUserData *ud = (RtUserData*) userData;
    
    (void) timeinfo; // Prevent unused variable warnings.
    (void) statusFlags;

    if (inputBuffer == NULL){
        for(unsigned long i = 0; i < framesPerBuffer; i++){
            *out++ = SAMPLE_SILENCE;  // left
            *out++ = SAMPLE_SILENCE;  // right
        }
        return paContinue;
    }

    for(unsigned long i = 0; i < framesPerBuffer; i++){
        SAMPLE inputSample = *in++; // mono input
        
        // no effect
        if (ud->effects->norm){
            *out++ = inputSample;  // left
            *out++ = inputSample;  // right
        }
        
        // tremolo effect
        else if (ud->effects->trem){
            double tremolo = (1.0 - ud->params->TREM_DEPTH) + ud->params->TREM_DEPTH * (0.5 * (1.0 + sin(ud->params->tremPhase)));
            
            ud->params->tremPhase += ud->tremIncrement;

            if (ud->params->tremPhase >= 2.0 * ud->params->PI) ud->params->tremPhase -= 2.0 * ud->params->PI;
            
            if (inputSample > 1.0f)  inputSample = 1.0f;
            if (inputSample < -1.0f) inputSample = -1.0f;

            *out++ = inputSample * tremolo;  // left
            *out++ = inputSample * tremolo;  // right
        }
        
        // delay effect
        else if (ud->effects->delay){
            SAMPLE delayedSample = ud->delayBuffer[ud->delayIndex];
            
            // Mix original and delayed signals
            SAMPLE outputSample =   (1.0 - ud->params->MIX) * inputSample
                                    + ud->params->MIX * delayedSample;
            
            // Store current input sample in delay buffer
            ud->delayBuffer[ud->delayIndex] = inputSample + delayedSample * ud->params->FEEDBACK;
            
            // Increment and wrap delay index
            ud->delayIndex = (ud->delayIndex + 1) % ud->delayBufferSize;
            
            if (outputSample > 1.0f)  outputSample = 1.0f;
            if (outputSample < -1.0f) outputSample = -1.0f;

            *out++ = outputSample;  // left
            *out++ = outputSample;  // right
        }
    }
    return paContinue;
}
#endif //REALTIME_CALLBACK_H