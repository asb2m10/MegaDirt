#pragma once

#include "Library.h"
#include "Dispatch.h"
#include <climits>

class DirtAudioProcessor;

struct DirtVoice { 
    int id;
    int serialId;

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
    float eventEnd;

    float panl = 1;
    float panr = 1;
    float gain = 1;

    juce::ADSR adsr;

    inline float getNextSample() {
        envPos++;
        if ( envPos > releasePos )
            adsr.noteOff();
        if ( envPos >= eventEnd )
            active = false;
            
        return adsr.getNextSample();
    }

    /*
     * @brief Tells if the voice should be rendered.
     */
    bool active = false;
};

class DirtSampler {
    std::array<DirtVoice, 30> voices;
    float sampleRate = 44100;

    float sampleLatency;
    const float EVENT_LATENCY = 0.3;
    void advance(int samples);

    void processVoice(DirtVoice &voice, juce::AudioBuffer<float> &buffer, int numSamples);

    double lastEvent;
    double lastSyncEvent;
public:
    DirtSampler() {
        juce::ADSR::Parameters envParameters(0.01,0,1,0.1);
        for(int i=0; i<voices.size(); i++) {
            voices[i].id = i;
            voices[i].adsr.setParameters(envParameters);
        }
    }

    void panic() {
        for (auto &voice: voices) {
            voice.samplePos = voice.sampleEnd;
        }
    }

    friend DirtAudioProcessor;

    int offset(int &sampleStart, Event *event);
    int offset(float cps, float cycle);
    
    void setSampleRate(float rate);
    void processBlock(juce::AudioBuffer<float> &buffer, int numSamples);
    void play(Event *event, Sample *sample, int offsetStart, int playLength);
};
