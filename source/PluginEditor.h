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
    virtual void timerCallback() override;

    juce::StringArray getMenuBarNames() {
        return juce::StringArray({"File", "Settings"});
    }

    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& str);
    void menuItemSelected(int x, int y);

private:

    enum MenuResults {
        configPath = 1000,
        forceOrbit0,
        enableDebug
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

    juce::ToggleButton syncHost;

    std::unique_ptr<juce::MenuBarComponent> menuBar;

    void setLibraryPath();

    juce::AlertWindow *settingsWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DirtAudioProcessorEditor)
};
