/*
 * audio.cpp
 *
 * Tiffany Liu
 * 9 February 2026
 * 
 * Contains function implementations for audio.h
*/

#include "../include/audio.h"

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
    snd_pcm_sw_params_malloc(&sw_params);
    snd_pcm_sw_params_current(*handle, sw_params);


    if (stream == SND_PCM_STREAM_PLAYBACK){
       snd_pcm_sw_params_set_start_threshold(*handle, sw_params, buffer - period);
       snd_pcm_sw_params_set_avail_min(*handle, sw_params, period);
    }

     err = snd_pcm_sw_params(*handle, sw_params);
    if (err < 0) {
        fprintf(stderr, "Error setting SW parameters: %s\n", snd_strerror(err));
        return err;
    }
    snd_pcm_sw_params_free(sw_params);

    return 0;
}


// initialize data
void initData(RtUserData &ud, AudioParams &audioParams, EffectChoices &effectChoice){
    ud.params = &audioParams;
    ud.effects = &effectChoice;
 
    ud.tremIncrement = 2.0 * audioParams.PI * audioParams.TREM_FREQ / (float)AudioParams::SAMPLE_RATE;
 
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


void stream(RtUserData &userData, AudioParams &audioParams,
                EffectChoices &effectChoice,
                snd_pcm_t *inHandle, snd_pcm_t *outHandle,
	            snd_pcm_uframes_t period,
                std::atomic<bool> &running){

    std::vector<SAMPLE> inputBlock(FRAMES_PER_BUFFER * audioParams.CHANNELS);
    std::vector<SAMPLE> outputBlock(FRAMES_PER_BUFFER * audioParams.CHANNELS);

    struct pollfd pfds[2];
    snd_pcm_poll_descriptors(inHandle, pfds, 1);
    snd_pcm_poll_descriptors(outHandle, pfds + 1, 1);

    while (running.load(std::memory_order_relaxed)){
        int ret = poll(pfds, 2, -1);
        if (ret <= 0)
            continue;
        snd_pcm_sframes_t framesRead =
        snd_pcm_readi(inHandle, inputBlock.data(), period);
        
        if (framesRead == -EAGAIN)
            continue;

        if (framesRead == -EPIPE) {     // xrun
            fprintf(stderr, "XRUN (capture)\n");
            snd_pcm_prepare(inHandle);
            continue;
        }

        if (framesRead < 0)
            continue;

        // *** process ***
        processBlock(
            inputBlock.data(),
            outputBlock.data(),
            framesRead,
            &userData
            );

        // write to output
        snd_pcm_sframes_t framesWritten =
            snd_pcm_writei(outHandle, outputBlock.data(), framesRead);

        if (framesWritten == -EPIPE) {   // xrun
            fprintf(stderr, "XRUN (playback)\n");
            snd_pcm_prepare(outHandle);
            continue;
        }
    }
    
    snd_pcm_drop(inHandle);
    snd_pcm_drop(outHandle);
    snd_pcm_prepare(inHandle);
    snd_pcm_prepare(outHandle);
}