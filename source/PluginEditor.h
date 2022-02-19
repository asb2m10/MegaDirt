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
    void resized() override;
    void playSound(juce::String soundName, int n);
    virtual void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DirtAudioProcessor &audioProcessor;
    juce::TreeView soundBrowser;
    juce::TextButton panicButton;
    juce::TextEditor showLog;
    int logLines = 0;
    RootTreeViewItem *rootItem;

    juce::Label libraryContent;
    juce::TextButton libraryPath;
    StatusBar statusBar;

    juce::ToggleButton debugEvent;
    juce::ToggleButton forceObrit0;
    juce::ToggleButton syncHost;

    void setLibraryPath();

    juce::AlertWindow *settingsWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirtAudioProcessorEditor)
};
