#include "DirtSampler.h"

inline void processVoice(DirtVoice &voice, juce::AudioBuffer<float> &buffer, int numSamples) {
    float* outL = buffer.getWritePointer (0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

    if ( voice.startOffset > 0 ) {
        voice.startOffset -= numSamples;

        if ( voice.startOffset >= 0 ) {
            return;
        }

        voice.startOffset *= -1;
        numSamples -= voice.startOffset;

        outL += voice.startOffset;
        if (outR != nullptr) {
            outR += voice.startOffset;
        } 
        voice.startOffset = 0;
    }

    const float* const inL = voice.sample->getBuffer().getReadPointer (0);
    const float* const inR = voice.sample->getBuffer().getNumChannels() > 1 ? voice.sample->getBuffer().getReadPointer(1) : nullptr;

    while (--numSamples >= 0) {
        auto pos = (int) voice.samplePos;
        auto alpha = (float) (voice.samplePos - pos);
        auto invAlpha = 1.0f - alpha;

        // just using a very simple linear interpolation here..
        float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
        float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha) : l;

        // TODO: process gain
        l *= 1;
        r *= 1;

        if (outR != nullptr) {
            *outL++ += l;
            *outR++ += r;
        } else {
            *outL++ += (l + r) * 0.5f;
        }

        voice.samplePos += voice.pitchRatio;

        if (voice.samplePos >= voice.sampleEnd ) {
            break;
        }
    }
}

void DirtSampler::processBlock(juce::AudioBuffer<float> &buffer, int numSamples) {
    for (auto &voice: voices) {
        if ( voice.samplePos < voice.sampleEnd ) {
            processVoice(voice, buffer, numSamples);
        }
    }
}

void DirtSampler::setSampleRate(float rate) {
    sampleRate = rate;
}

void DirtSampler::play(Sample *sample, int sampleStart, float begin, float end) {
    int length = sample->getLength();
    
    for (auto &voice: voices) {
        // TODO: legato
        /*
        if ( voice.sample == sample ) {
            voice.samplePos = 0;
            return;
        }*/

        if ( voice.samplePos >= voice.sampleEnd ) {
            voice.startOffset = sampleStart;
            voice.sample = sample;
            voice.samplePos = 0;
            voice.sampleEnd = sample->getLength();
            voice.pitchRatio = sample->getSampleRate() / sampleRate;
            return;
        }

        // TODO: voice steal*/
    }
    //printf("voice full\n");
}