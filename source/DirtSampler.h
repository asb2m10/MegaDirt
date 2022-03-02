#pragma once

#include <climits>
#include <juce_dsp/juce_dsp.h>
#include "Library.h"
#include "Dispatch.h"
#include "DelayFx.h"

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
    //float gain = 1;

    int loop;

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

class DirtFX {
    float sampleRate;
public:
    juce::dsp::LadderFilter<float> filter;
    Delay<float> delay;
    juce::dsp::Reverb reverb;

    int bitcrush;

    void reset() {
        bitcrush = 65535;
        filter.reset();
        filter.setEnabled(false);
        delay.reset();
        reverb.reset();
        reverb.setEnabled(false);
    }

    void prepareToPlay(double rate, int samplesPerBlock) {
        juce::dsp::ProcessSpec spec;

        spec.numChannels = 2;
        spec.sampleRate = rate;
        spec.maximumBlockSize = samplesPerBlock;

        filter.prepare(spec);
        delay.prepare(spec);
        reverb.prepare(spec);
        reset();

        sampleRate = rate;
    }

    void apply(Event *e);

    template <typename ProcessContext>
    void process(const ProcessContext& context);
};

class DirtSampler {
    std::array<DirtVoice, 30> voices;
    float sampleRate = 44100;
    void processVoice(DirtVoice &voice, juce::AudioBuffer<float> &buffer, int numSamples);
    std::array<DirtFX, 4> fx;
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
        for (auto &i : fx) {
            i.reset();
        }
    }

    friend DirtAudioProcessor;

    int offset(int &sampleStart, Event *event);
    int offset(float cps, float cycle);
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float> &buffer, int numSamples);
    void play(Event *event, Sample *sample, int offsetStart, int playLength);
};
