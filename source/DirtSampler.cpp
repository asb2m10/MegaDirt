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
                if ( voice.loop-- < 1 ) {
                    voice.active = false;
                    break;
                } 
                voice.samplePos = voice.sampleEnd;
            }
        } else {
            if (voice.samplePos >= voice.sampleEnd ) {
                if ( voice.loop-- < 1 ) {
                    voice.active = false;
                    break;
                }
                voice.samplePos = voice.sampleStart;
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

    for(int i=0; i<fx.size();i++) {
        if ( buffer.getNumChannels() <= i*2 )
            break;
        auto inoutBlock = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock(i*2, 2);
        fx[i].process(juce::dsp::ProcessContextReplacing<float>(inoutBlock));
    }
}

void DirtSampler::prepareToPlay(double rate, int samplesPerBlock) {
    sampleRate = sampleRate;
    for (auto &voice: voices) {
        voice.adsr.setSampleRate(rate);
    }

    for(auto &i : fx) {
        i.prepareToPlay(rate, samplesPerBlock);
    }
}

void DirtFX::apply(Event *event) {
    if ( reverb.isEnabled() || event->hasKey("room") ) {
        reverb.setEnabled(true);
        juce::Reverb::Parameters parms;
        parms.roomSize = event->get("size", 0.5);
        parms.wetLevel = event->get("room", 0);
        // TODO: support msg #dry
        reverb.setParameters(parms);
    }

    if ( event->hasKey("cutoff") ) {
        filter.setMode(juce::dsp::LadderFilterMode::LPF24);
        filter.setCutoffFrequencyHz(event->keys["cutoff"]);
        filter.setResonance(event->get("resonance", 0));
        filter.setEnabled(true);
    } else if ( event->hasKey("hcutoff") ) {
        filter.setMode(juce::dsp::LadderFilterMode::HPF24);
        filter.setCutoffFrequencyHz(event->keys["hcutoff"]);
        filter.setResonance(event->get("hresonance", 0));
        filter.setEnabled(true);
    } else if ( event->hasKey("bandf") ) {
        filter.setMode(juce::dsp::LadderFilterMode::BPF24);
        filter.setCutoffFrequencyHz(event->keys["bandf"]);
        filter.setResonance(event->get("bandq", 0));
        filter.setEnabled(true);
    } else if ( event->hasKey("djf") ) {
        float v = event->get("djf");
        // TODO: theses values must be scaled.
        if ( v < 0.5 ) {
            filter.setMode(juce::dsp::LadderFilterMode::LPF24);
            filter.setCutoffFrequencyHz(v * sampleRate);
        } else {
            filter.setMode(juce::dsp::LadderFilterMode::HPF24);
            filter.setCutoffFrequencyHz((1-v) * sampleRate);
        }
        filter.setResonance(0);
        filter.setEnabled(true);
    } else {
        filter.setEnabled(false);
    }
}

void DirtSampler::play(Event *event, Sample *sample, int offsetStart, int playLength) {    
    for (auto &voice: voices) {
        if ( ! voice.active ) {
            voice.active = true;
            voice.serialId = event->serialId;
            voice.startOffset = offsetStart;
            voice.sample = sample;

            voice.samplePos = event->get("begin", 0) * sample->getLength();
            voice.sampleEnd = event->get("end", 1) * sample->getLength();

            float speed = event->get("speed", 1);
            switch ( event->unit ) {
                case 'c' :
                    voice.pitchRatio = (voice.sampleEnd - voice.samplePos) / playLength;
                    voice.pitchRatio *= speed;
                break; 

                case 's':
                    if ( speed != 1 )
                        voice.pitchRatio = (voice.sampleEnd - voice.samplePos) / speed * sampleRate;
                    else
                        voice.pitchRatio = 1;
                break;

                default:
                    voice.pitchRatio = std::pow(2.0, (event->note / 12.0)) * speed;
                    voice.pitchRatio *= sample->getSampleRate() / sampleRate;
            }

            if ( voice.pitchRatio < 0 ) {
                voice.samplePos = voice.sampleEnd;
            }

            voice.loop = event->get("loop", 1);
            voice.loop = voice.loop < 1 ? 0 : voice.loop - 1;

            if ( playLength == 0 )
                playLength = abs(sample->getLength() * voice.pitchRatio);
            voice.eventEnd = playLength;

            voice.gain = event->get("gain", 1);
            float pan = event->get("pan", 0.5);
            voice.panl = pan < 0.5 ? 1 : 2 - pan * 2;
            voice.panr = pan > 0.5 ? 1 : pan * 2;
            voice.envPos = 0;
            voice.releasePos = playLength - 100;
            voice.orbit = event->orbit * 2;

            if ( fx.size() > event->orbit )
                fx[event->orbit].apply(event);

            voice.adsr.reset();
            voice.adsr.noteOn();
            return;
        }

        // TODO: voice steal
    }
    juce::Logger::writeToLog("voice full");
}
