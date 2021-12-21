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

class StatusBar : public juce::Component {
public:
    std::bitset<16> *orbitActivity;
    std::bitset<16> *midiActivity;

    void paint (juce::Graphics& g) override {
        g.fillAll(findColour(juce::TextEditor::backgroundColourId));

        int i = 0;
        juce::String display;

        for(int i=0; i<orbitActivity->size(); i++) {
            if ( orbitActivity->test(i) ) {
                display += "d" + juce::String(i+1) + " " ;
            }
        }

        for(int i=0; i<midiActivity->size(); i++) {
            if ( midiActivity->test(i) ) {
                display += "m" + juce::String(i+1) + " " ;
            }
        }

        orbitActivity->reset();
        midiActivity->reset();

        if ( display == "" ) 
            return;

        g.setColour(findColour(juce::Label::textColourId));
        g.drawText (display, 0, 0, getWidth() - 4, getHeight() - 3, juce::Justification::centredRight, true);
    }
};
