#include "DirtSampler.h"

void DirtSampler::processVoice(DirtVoice &voice, juce::AudioBuffer<float> &buffer, int numSamples) {
    int startOutput = 0;

    if ( voice.startOffset > 0 ) {
        if ( voice.startOffset > numSamples ) {
            voice.startOffset -= numSamples;
            return;
       }

        startOutput = voice.startOffset;
        numSamples -= startOutput;
        voice.startOffset = 0;
    }

    const float* const inL = voice.sample->getBuffer().getReadPointer (0);
    const float* const inR = voice.sample->getBuffer().getNumChannels() > 1 ? voice.sample->getBuffer().getReadPointer(1) : nullptr;
    float* outL = buffer.getWritePointer(voice.orbit, startOutput);
    float* outR = buffer.getWritePointer(voice.orbit+1, startOutput);
    int interpolationPos = voice.pitchRatio < 0 ? -1 : +1;

    while (--numSamples >= 0) {
        auto pos = (int) voice.samplePos;
        auto alpha = (float) (voice.samplePos - pos);
        auto invAlpha = 1.0f - alpha;

        // just using a very simple linear interpolation here..
        float l = (inL[pos] * invAlpha + inL[pos + interpolationPos] * alpha);
        float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + interpolationPos] * alpha) : l;

        float envValue = voice.getNextSample() * voice.gain;

        l *= (envValue * voice.panl);
        r *= (envValue * voice.panr);

        if (outR != nullptr) {
            *outL++ += l;
            *outR++ += r;
        } else {
            *outL++ += (l + r) * 0.5f;
        }

        voice.samplePos += voice.pitchRatio;

        if ( voice.pitchRatio < 0 ) {
            if (voice.samplePos < voice.sampleStart ) {
                voice.active = false;
                break;
            }
        } else {
            if (voice.samplePos >= voice.sampleEnd ) {
                voice.active = false;
                break;
            }
        }
    }
}

void DirtSampler::processBlock(juce::AudioBuffer<float> &buffer, int numSamples) {
    for (auto &voice: voices) {
        if ( voice.active ) {
            processVoice(voice, buffer, numSamples);
        }
    }
}

void DirtSampler::setSampleRate(float rate) {
    sampleRate = rate;
    sampleLatency = sampleRate * EVENT_LATENCY;
    for (auto &voice: voices) {
        voice.adsr.setSampleRate(rate);
    }
}

void DirtSampler::play(Event *event, Sample *sample, int offsetStart, int playLength) {    
    for (auto &voice: voices) {
        if ( ! voice.active ) {
            voice.active = true;
            voice.serialId = event->serialId;
            voice.startOffset = offsetStart;
            voice.sample = sample;


            if (event->speed != 1)
                voice.pitchRatio = event->speed;
            else
                voice.pitchRatio = std::pow(2.0, (event->note / 12.0));
            voice.pitchRatio *= sample->getSampleRate() / sampleRate;

            if ( voice.pitchRatio < 0 ) {
                voice.samplePos = event->end * sample->getLength();
                voice.sampleEnd = event->begin * sample->getLength();
            } else {
                voice.samplePos = event->begin * sample->getLength();
                voice.sampleEnd = event->end * sample->getLength();
            }

            if ( playLength == 0 )
                playLength = abs(sample->getLength() * voice.pitchRatio);
            voice.eventEnd = playLength;

            voice.gain = event->gain;
            voice.panl = event->pan < 0.5 ? 1 : 2 - event->pan * 2;
            voice.panr = event->pan > 0.5 ? 1 : event->pan * 2;
            voice.envPos = 0;
            voice.releasePos = playLength - 100;
            voice.orbit = event->orbit * 2;
            voice.adsr.reset();
            voice.adsr.noteOn();
            return;
        }

        // TODO: voice steal
    }
    juce::Logger::writeToLog("voice full");
}

void DirtSampler::advance(int samples) {
    syncSamplePos += samples;
}

int DirtSampler::offset(float cps, float cycle) {
    if ( cps == 0 )
        return 0;

    double dest = (sampleRate / cps) * cycle;
    //double recycle = (syncSamplePos - sampleLatency)/ sampleRate * 0.5625;
    //printf("pos %f-%f ", recycle, event->cycle);

    if ( dest < syncSamplePos ) {
        printf(" ** Sample to soon %f %f delta %f\n", syncSamplePos, dest, dest - syncSamplePos);
        syncSamplePos = dest - sampleLatency;
        return sampleLatency;
    } else if (  dest > syncSamplePos + sampleLatency * 2) {
        printf(" ** Sample to far %f %f delta %f\n", syncSamplePos, dest, dest - syncSamplePos);
        syncSamplePos = dest;
        return sampleLatency;
    }

    lastSyncEvent = dest;
    return dest - syncSamplePos;
}
