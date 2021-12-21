/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

//#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "DirtUI.h"

//==============================================================================
/**
 */
class DirtAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer {
public:
    DirtAudioProcessorEditor(DirtAudioProcessor &);
    ~DirtAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;

    void playSound(juce::String soundName, int note);

    virtual void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DirtAudioProcessor &audioProcessor;
    juce::TreeView soundBrowser;
    juce::TextButton panicButton;
    juce::TextEditor showLog;
    RootTreeViewItem *rootItem;

    juce::Label libraryContent;
    juce::TextButton libraryPath;

    void setLibraryPath();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirtAudioProcessorEditor)
};
