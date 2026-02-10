/*
 * Engine.cpp
 *
 * Tiffany Liu
 * 9 February 2026
*/

#include "../include/Engine.h"

Engine::Engine(){
    period = FRAMES_PER_BUFFER;
    buffer = FRAMES_PER_BUFFER * BUFFER_MULT;

    if (setupPCM(DEVICE_NAME, &inHandle,
                 SND_PCM_STREAM_CAPTURE,
                 2, audioParams.SAMPLE_RATE,
                 period, buffer) < 0){
        throw std::runtime_error("Failed to setup input PCM")
    }

    if (setupPCM(DEVICE_NAME, &outHandle,
                 SND_PCM_STREAM_PLAYBACK,
                 2, audioParams.SAMPLE_RATE,
                 period, buffer) < 0){
        throw std::runtime_error("Failed to setup output PCM")
    }

    snd_pcm_nonblock(inHandle, 1);
    snd_pcm_nonblock(outHandle, 1);
}


Engine::~Engine(){
    stop();
    if (inHandle){
        snd_pcm_close(inHandle);
        inHandle = nullptr;
    }

    if (outHandle){
        snd_pcm_close(outHandle);
        outHandle = nullptr;
    }
}


void Engine::start(){
    if (running.load()) return;
    initData(userData, audioParams, effectChoice);
    streamLoop();
}


void Engine::stop(){
    running.store(false);
    if (audioThread.joinable())
        audioThread.join();
}


void Engine::streamLoop(){
    running.store(true);

    audioThread = std::thread([this] {
        // only one thread at a time
        std::lock_guard<std::mutex> lock(paramMutex);

        stream(userData, audioParams,
            effectChoice, inHandle,
            outHandle, period,
            running);
    });
}


void Engine::setEffect(EffectChoices effect){
    std::lock_guard<std::mutex> lock(paramMutex);
    // TODO: change effectChoice strucutre in types.h
    effectChoice = effect;
}

void Engine::setMix(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.MIX = v;
}

void Engine::setTremFreq(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.TREM_FREQ = v;
}

void Engine::setTremDepth(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.TREM_DEPTH = v;
}

void Engine::setTremPhase(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.TREM_PHASE = v;
}

void Engine::setDelay(int v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.DELAY_MS = v;
}

void Engine::setFeedback(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.FEEDBACK = v;
}

void Engine::setReverbTaps(int v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.REVERB_TAPS = v;
}

void Engine::setReverbDecay(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.REVERB_DECAY = v;
}

void Engine::setDownSampleRate(int v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.DOWNSAMPLE_RATE = v;
}

void Engine::setBitDepth(int v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.BIT_DEPTH = v;
}

void Engine::setOdDrive(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.OD_DRIVE = v;
}

void Engine::setOdTone(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.OD_TONE = v;
}

void Engine::setDistDrive(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.DIST_DRIVE = v;
}

void Engine::setDistTone(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.DIST_TONE = v;
}

void Engine::setFuzzDrive(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.FUZZ_DRIVE = v;
}

void Engine::setFuzzTone(float v){
    std::lock_guard<std::mutex> lock(paramMutex);
    audioParams.FUZZ_TONE = v;
}
