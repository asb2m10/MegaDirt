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

    Library library;

    friend DirtAudioProcessorEditor;

    const int DIRT_UPD_PORT = 57120;
private:
    juce::AudioParameterFloat *gain;
    Dispatch dispatch;
    DirtSampler sampler;

    juce::ApplicationProperties appProp;

    const int DEFAULT_MIDI_VELOCITY = 100;

    std::array<Orbit, 16> orbits;

    std::bitset<16> orbitActivity;
    std::bitset<16> midiActivity;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirtAudioProcessor)
};
