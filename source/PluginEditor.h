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
class DirtAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer, public juce::MenuBarModel {
public:
    DirtAudioProcessorEditor(DirtAudioProcessor &);
    ~DirtAudioProcessorEditor() override;

    //==============================================================================
    void resized() override;
    void playSound(juce::String soundName, int n);
    void timerCallback() override;
    void paint(juce::Graphics &g) override;

    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& str);
    void menuItemSelected(int x, int y);

    juce::StringArray getMenuBarNames() {
        return juce::StringArray({"File", "Settings"});
    }

private:

    enum MenuResults {
        configPath = 1000,
        forceOrbit0,
        enableDebug,
        schedOffset
    };

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DirtAudioProcessor &audioProcessor;
    juce::TreeView soundBrowser;
    juce::TextButton panicButton;
    LogViewer logViewer;

    int logLines = 0;
    RootTreeViewItem *rootItem;

    juce::Label libraryContent;
    StatusBar statusBar;

    std::unique_ptr<juce::MenuBarComponent> menuBar;

    void setLibraryPath();

    std::unique_ptr<juce::AlertWindow> modalWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirtAudioProcessorEditor)
};
