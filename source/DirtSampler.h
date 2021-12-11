#pragma once

#include "Library.h"
#include "Dispatch.h"
#include <climits>

struct DirtVoice { 
    int id;

    Sample *sample;
    float samplePos = 1;
    float sampleStart = 0;
    float sampleEnd = 0;
    float pitchRatio = 0;
    int startOffset = 0;
    int orbit = 0;



    float getEnv() {

    }
};


/**
 * To be able to sync TidalCycles with DAW timeline.
 */
class DirtSync {
    void reset(double target) {
        printf("resync***\n");
        samplePos = target;
    }
public:
    double samplePos = 0;
    float sampleRate;
    float sampleLatency;
    const float EVENT_LATENCY = 0.3;

    int oldDiff;

    int offset(float cps, float cycle) {
        double dest = (sampleRate / cps) * cycle;
        double recycle = (samplePos - sampleLatency)/ sampleRate * 0.5625;
        printf("pos %f-%f ", recycle, cycle);

        if ( dest < samplePos ) {
            printf(" ** Sample to soon %f %f delta %f\n", samplePos, dest, dest - samplePos);
            samplePos = dest - sampleLatency;
            return sampleLatency;
        }

        if (  dest > samplePos + sampleLatency * 2) {
            printf(" ** Sample to far %f %f delta %f\n", samplePos, dest, dest - samplePos);
            samplePos = dest;
            return sampleLatency;
        }
    
        int target = dest - samplePos;

        printf("ok diff %f diffdiff %i\n", recycle-cycle, target);
        oldDiff = target;
        return target;
    }

    int delta(float cps, float cycle, float delta) {
        double dest = (sampleRate / cps) * cycle;
        return (dest - samplePos) + sampleRate * delta;
    }

    int bloc = 0;
    void advance(int samples) {
        samplePos += samples;
        if ( samples != bloc ) {
            printf("bloc size %i sample\n", samples);
            bloc = samples;
        }
    }

    void setSampleRate(float rate) {
        sampleRate = rate;
        sampleLatency = sampleRate * EVENT_LATENCY;
    }
};

class DirtSampler {
    std::array<DirtVoice, 30> voices;
    float sampleRate = 44100;

public:

    DirtSampler() {
        for(int i=0; i<voices.size();i++) {
            voices[i].id = i;
        }
    }

    void setSampleRate(float rate);
    void processBlock(juce::AudioBuffer<float> &buffer, int numSamples);
    void play(Sample *sample, int sampleStart = 0, float begin = 0, float end = -1);
};
