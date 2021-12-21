#pragma once

#include "PluginProcessor.h"

class NoteTreeViewItem : public juce::TreeViewItem {
    juce::String label;
    juce::String *sound;
    int idx;
public:
    NoteTreeViewItem(int idx, juce::String *sound, juce::String label) : idx(idx), sound(sound), label(label)  {}

    juce::String getUniqueName() const {
        return *sound;
    }

    bool mightContainSubItems() {
        return false;
    }

    void paintItem(juce::Graphics& g, int width, int height) {
        if ( isSelected() )
            g.fillAll (getOwnerView()->findColour(juce::ComboBox::outlineColourId));
        g.setColour(getOwnerView()->findColour(juce::Label::textColourId));
        g.setFont(height * 0.7f);
        g.drawText (label, 4, 0, width - 4, height, juce::Justification::centredLeft, true);
    }

    void itemClicked(const juce::MouseEvent &e);
};

class SoundTreeViewItem : public juce::TreeViewItem {
    juce::String soundName;
    Library *library;
public:
    SoundTreeViewItem(juce::String name, Library *lib) : soundName(name), library(lib) {}

    juce::String getUniqueName() const {
        return soundName;
    }

    void itemOpennessChanged(bool isNowOpen) {
        if ( isNowOpen ) {
            juce::Array<SampleHolder> &samples = library->content.getReference(soundName);
            for(int i=0;i<samples.size();i++) {
                addSubItem(new NoteTreeViewItem(i, &soundName, soundName + ":" + juce::String(i) + juce::String(" / ") + samples[i].filename.getFileNameWithoutExtension() ));
            }
        } else {
            clearSubItems();
        }
    }
    
    bool mightContainSubItems() {
        return true;
    }

    void paintItem (juce::Graphics& g, int width, int height) {
        g.setColour(getOwnerView()->findColour(juce::Label::textColourId));
        g.setFont(height * 0.7f);
        g.drawText (soundName, 4, 0, width - 4, height, juce::Justification::centredLeft, true);
    }    
};

class RootTreeViewItem : public juce::TreeViewItem {
    Library *library;
public: 
    juce::HashMap<juce::String, int> refContent;

    RootTreeViewItem(Library *lib) : library(lib) {}

    bool mightContainSubItems() {
        return true;
    }

    void refresh();
};

class OrbitViewer : public juce::Component {


};

class StatusBar : juce::Component {
public:
    void paint (juce::Graphics& g) override {

    }
};
