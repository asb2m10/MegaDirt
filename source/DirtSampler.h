#pragma once

#include "Library.h"
#include "Dispatch.h"
#include <climits>

struct DirtVoice { 
    int id;

    Sample *sample;

    /**
     * @brief Position of the voice on the sample
     */
    float samplePos = 1;

    /**
     * @brief Position on the sample where the voice should start playing
     */
    float sampleStart = 0;

    /**
     * @brief Position on the sample where the voice should stop playing
     */
    float sampleEnd = 0;

    /**
     * @brief Pitch ratio on the playhead
     */
    float pitchRatio = 0;

    /**
     * @brief The moment the voice should start playing
     */
    int startOffset = 0;

    /**
     * @brief The "output" orbit of the voice.
     */
    int orbit = 0;

    // Enveloppe
    float envPos;
    float releasePos;

    float pan = 0.5;
    float gain = 1;

    juce::ADSR adsr;

    inline float getNextSample() {
        if ( envPos > releasePos )
            adsr.noteOff();
        return adsr.getNextSample();
    }
};

class DirtSampler {
    std::array<DirtVoice, 30> voices;
    float sampleRate = 44100;

    double syncSamplePos = 0;
    float sampleLatency;
    const float EVENT_LATENCY = 0.3;
    void advance(int samples);

public:
    DirtSampler() {
        for(int i=0; i<voices.size();i++) {
            juce::ADSR::Parameters envParameters(0.01,0,1,0.1);
            voices[i].id = i;
            voices[i].adsr.setParameters(envParameters);
        }
    }

    void panic() {
        for (auto &voice: voices) {
            voice.samplePos = voice.sampleEnd;
        }
    }

    int offset(int &sampleStart, Event *event);

    void setSampleRate(float rate);
    void processBlock(juce::AudioBuffer<float> &buffer, int numSamples);
    void play(Event *event, Sample *sample, int offsetStart, int playLength);
};
