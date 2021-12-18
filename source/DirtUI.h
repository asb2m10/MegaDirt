#pragma once

#include "PluginProcessor.h"

class TreeViewSorter {
public:
    static int compareElements(juce::TreeViewItem *i1, juce::TreeViewItem *i2) {
        return (i1->getUniqueName() < i2->getUniqueName()) ? -1 : ((i1->getUniqueName() < i2->getUniqueName()) ? 1 : 0);
    }
};
const TreeViewSorter treeViewSorter;

class NoteTreeViewItem : public juce::TreeViewItem {
    juce::String label;
    juce::String *sound;
    int idx;
public:

    NoteTreeViewItem(int idx, juce::String *sound, juce::String label) : idx(idx), sound(sound), label(label)  {}

    juce::String getUniqueName() {
        return *sound;
    }

    bool mightContainSubItems() {
        return false;
    }

    void paintItem(juce::Graphics& g, int width, int height) {
        // if this item is selected, fill it with a background colour..
        if (isSelected())
            g.fillAll (juce::Colours::blue.withAlpha (0.3f));
        // use a "colour" attribute in the xml tag for this node to set the text colour..
        //g.setColour();
        g.setFont(height * 0.7f);
        // draw the xml element's tag name..
        g.drawText (label, 4, 0, width - 4, height, juce::Justification::centredLeft, true);
    }

    void itemClicked(const juce::MouseEvent &e);
};

class SoundTreeViewItem : public juce::TreeViewItem {
    juce::String soundName;
    Library *library;
public:
    SoundTreeViewItem(juce::String name, Library *lib) : soundName(name), library(lib) {}

    juce::String getUniqueName() {
        return soundName;
    }

    void itemOpennessChanged(bool isNowOpen) {
        if ( isNowOpen ) {
            juce::Array<SampleHolder> &samples = library->content.getReference(soundName);
            for(int i=0;i<samples.size();i++) {
                addSubItem(new NoteTreeViewItem(i, &soundName, soundName + juce::String(i) + juce::String(" / ") + samples[i].filename.getFileNameWithoutExtension() ));
            }
        } else {
            clearSubItems();
        }
    }
    
    bool mightContainSubItems() {
        return true;
    }

    void paintItem (juce::Graphics& g, int width, int height) {
        // if this item is selected, fill it with a background colour..
        if (isSelected())
            g.fillAll (juce::Colours::blue.withAlpha (0.3f));
        // use a "colour" attribute in the xml tag for this node to set the text colour..
        //g.setColour();
        g.setFont(height * 0.7f);
        // draw the xml element's tag name..
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

    void refresh() {
        for(juce::HashMap<juce::String, juce::Array<SampleHolder>>::Iterator i(library->content); i.next();) {
            if ( ! refContent.contains(i.getKey()) ) {
                refContent.set(i.getKey(), 0);
                SoundTreeViewItem *sound = new SoundTreeViewItem(i.getKey(), library);
                addSubItemSorted(treeViewSorter, sound);
            }
        }
    }
};

class OrbitViewer : public juce::Component {


};

class StatusBar : juce::Component {
public:
    void paint (juce::Graphics& g) override {

    }
};
