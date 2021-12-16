/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
DirtAudioProcessorEditor::DirtAudioProcessorEditor(DirtAudioProcessor &p) : 
    AudioProcessorEditor(&p), audioProcessor(p), soundBrowser("SoundBrowser"), panicButton("Panic") {

    addAndMakeVisible(panicButton);
    addAndMakeVisible(soundBrowser);
    setSize(700, 400);

}

DirtAudioProcessorEditor::~DirtAudioProcessorEditor() {
  // stopTimer();
}

void DirtAudioProcessorEditor::timerCallback() {}

//==============================================================================
void DirtAudioProcessorEditor::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a
    // solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    /*g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);*/
}

void DirtAudioProcessorEditor::resized() {
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    int width = getWidth();
    int height = getHeight();
    panicButton.setBounds(width-55, 5, 50, 30);
    soundBrowser.setBounds(5,5,300,300);
}
