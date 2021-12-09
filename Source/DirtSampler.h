
#pragma once

#include "Library.h"
#include "Dispatch.h"
#include <climits>

struct DirtVoice { 
    int id;

    Sample *sample;
    float samplePos = 1;
    float sampleEnd = 0;
    float sampleStart = 0;
    float pitchRatio = 0;
    int startOffset = 0;
};

class TimeTracker {
    float internalCycle;
    float sampleRate;
public:

    TimeTracker() {
    }

    int offset(float cps, float cycle) {
        double dest = (sampleRate / cps) * cycle;
    
      
    }

    void advance(int samples) {
        //pos += samples;
    }
};

class DirtSampler {
    std::array<DirtVoice, 30> voices;
public:
    float sampleRate = 44100;

    DirtSampler() {
        for(int i=0; i<voices.size();i++) {
            voices[i].id = i;
        }
    }

    void processBlock(juce::AudioBuffer<float> &buffer, int numSamples);
    void play(Sample *sample, int sampleStart = 0, float begin = 0, float end = -1);
};
