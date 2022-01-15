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
    float* outL = buffer.getWritePointer (0, startOutput);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1, startOutput) : nullptr;

    while (--numSamples >= 0) {
        /*if ( voice.samplePos == voice.sampleStart ) {
            double dest = syncSamplePos + (buffer.getNumSamples() - numSamples);
            printf("%i %f delta %f == %f\n", voice.serialId, syncSamplePos, dest - lastEvent, dest-lastSyncEvent);
            lastEvent = dest;
        }*/

        auto pos = (int) voice.samplePos;
        auto alpha = (float) (voice.samplePos - pos);
        auto invAlpha = 1.0f - alpha;

        // just using a very simple linear interpolation here..
        float l = (inL[pos] * invAlpha + inL[pos + 1] * alpha);
        float r = (inR != nullptr) ? (inR[pos] * invAlpha + inR[pos + 1] * alpha) : l;

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

        if (voice.samplePos >= voice.sampleEnd )
            break;
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
    sampleLatency = sampleRate * EVENT_LATENCY;
    for (auto &voice: voices) {
        voice.adsr.setSampleRate(rate);
    }
}

void DirtSampler::play(Event *event, Sample *sample, int offsetStart, int playLength) {    
    for (auto &voice: voices) {
        if ( voice.samplePos >= voice.sampleEnd ) {
            voice.serialId = event->serialId;
            voice.startOffset = offsetStart;
            voice.sample = sample;
            voice.samplePos = event->begin * sample->getLength();
            voice.sampleEnd = event->end * sample->getLength();
            voice.pitchRatio = std::pow(2.0, (event->note / 12.0)) * sample->getSampleRate() / sampleRate;

            if ( playLength == 0 )
                playLength = sample->getLength() * voice.pitchRatio;

            voice.gain = event->gain;
            voice.panl = event->pan < 0.5 ? 1 : 2 - event->pan * 2;
            voice.panr = event->pan > 0.5 ? 1 : event->pan * 2;
            voice.envPos = 0;
            voice.releasePos = playLength - 100;
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

// int DirtSampler::offset(int &sampleStart, Event *event) {
//     if ( event->cps == 0 ) {
//         sampleStart = 0;
//         return 0;
//     }

//     double dest = (sampleRate / event->cps) * event->cycle;
//     double recycle = (syncSamplePos - sampleLatency)/ sampleRate * 0.5625;
//     //printf("pos %f-%f ", recycle, event->cycle);

//     if ( dest < syncSamplePos ) {
//         printf(" ** Sample to soon %f %f delta %f\n", syncSamplePos, dest, dest - syncSamplePos);
//         syncSamplePos = dest - sampleLatency;
//         sampleStart = sampleLatency;
//     } else if (  dest > syncSamplePos + sampleLatency * 2) {
//         printf(" ** Sample to far %f %f delta %f\n", syncSamplePos, dest, dest - syncSamplePos);
//         syncSamplePos = dest;
//         sampleStart = sampleLatency;
//     } else {
//         int target = dest - syncSamplePos;
//         //printf("ok diff %f diffdiff %i\n", recycle-event->cycle, target);
//         sampleStart = target;
//     }

//     return (dest - syncSamplePos) + sampleRate * event->delta;
// }

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
