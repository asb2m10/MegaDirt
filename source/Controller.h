#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

/**
 * @brief Alias are used to map SuperDirt synth parameteres to midi cc.
 */
class Alias {
public:
    struct AliasDef {
        int channel;
        int cc;
    };
    juce::HashMap<juce::String, AliasDef> map;
};

class TidalCtrl : public juce::AudioParameterFloat {
    juce::OSCSender *tidalSender;
    int cc;
public:
    TidalCtrl(juce::OSCSender *sender, int id) : AudioParameterFloat(juce::String("cF:") + juce::String(id), juce::String("ctrl/") + juce::String(id),
                0, 127, 0) {
        tidalSender = sender;
        cc = id;
    }

    void valueChanged(float newValue) override {
        juce::OSCMessage message("/ctrl");
        message.addString(juce::String(cc));
        message.addFloat32(newValue);
        tidalSender->send(message);
    }
};