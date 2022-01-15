/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <bitset>

#include "Dispatch.h"
#include "Library.h"
#include "DirtSampler.h"

class DirtAudioProcessorEditor;

// Dirty cheap logger
class DirtLogger : public juce::Logger {
public:
    juce::StringArray content;
    void printf(const char *fmt, ...);
    void logMessage(const juce::String &message) override {
        if (content.size() > 4096) 
            content.removeRange(0, 2048);
        content.add(message + "\n");
    }
};

//==============================================================================
/**
 */
class DirtAudioProcessor : public juce::AudioProcessor {
public:
    //==============================================================================
    DirtAudioProcessor();
    ~DirtAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    //==============================================================================
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String &newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock &destData) override;
    void setStateInformation(const void *data, int sizeInBytes) override;

    struct Orbit {
        bool activity;
        int targetBus = 0;
    };

    friend DirtAudioProcessorEditor;

    const int DIRT_UDP_PORT = 57120;
private:
    juce::AudioParameterFloat *gain;
    Library library;
    Dispatch dispatch;
    DirtSampler sampler;

    juce::ApplicationProperties appProp;
    
    std::array<Orbit, 16> orbits;

    std::bitset<16> orbitActivity;
    std::bitset<16> midiActivity;

    void panic();

    bool isActive;

    juce::Array<Event *> pendingEv;
    DirtLogger logger;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirtAudioProcessor)
};
