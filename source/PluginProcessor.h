/*
    MegaDirt Copyright (c) 2025 Pascal Gauthier.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <bitset>

#include "Dispatch.h"
#include "Library.h"
#include "DirtSampler.h"
#include "Controller.h"
#include "Model.h"
#include "TidalRunner.h"
#include "juce_gui_extra/juce_gui_extra.h"

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

    friend DirtAudioProcessorEditor;

    const int DIRT_UDP_PORT = 57120;
    juce::ValueTree rootValueTree;

private:
    bool panicMode = false;
    Library library;
    Dispatch dispatch;
    DirtSampler sampler;

    juce::ApplicationProperties appProp;

    std::bitset<16> patternActivity;
    std::bitset<16> midiActivity;

    void panic() {
        panicMode = true;
    }

    juce::Array<Event *> pendingEv;
    DirtLogger logger;

    bool debugEvent = false;
    bool forceOrbit0 = true;
    juce::CachedValue<int> scheduleOffset;

    void setSamplePath(juce::String paths, bool lazyLoading);

    double lastEvent;

    Alias aliases;
    juce::OSCSender tidalSender;

    TidalRunner tidalRunner;
    juce::File tidalBootScript;
    juce::ValueTree pluginState;
    juce::CodeDocument codeDocument;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirtAudioProcessor)
};
