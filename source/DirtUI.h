#pragma once

#include "PluginProcessor.h"
#include "Controller.h"

class NoteTreeViewItem : public juce::TreeViewItem {
    juce::String label;
    juce::String *sound;
    SampleHolder *holder;
    int idx;
public:
    NoteTreeViewItem(int idx, juce::String *sound, SampleHolder *holder) : idx(idx), sound(sound), holder(holder)  {
        label = *sound + ":" + juce::String(idx) + juce::String(" / ") + holder->filename.getFileNameWithoutExtension();
    }

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

    juce::String getTooltip() override {
        Sample *sample = holder->sample.get();
        float sampleLengthSec = sample->getLength() / sample->getSampleRate();
        return juce::String(sample->getSampleRate()) + " / " + juce::String(sampleLengthSec);
    }

    void itemClicked(const juce::MouseEvent &e);
};

class SoundTreeViewItem : public juce::TreeViewItem {
    juce::String soundName;
    juce::Array<SampleHolder> *samples;
public:
    SoundTreeViewItem(juce::String name, juce::Array<SampleHolder> *samples) : soundName(name), samples(samples) {}

    juce::String getUniqueName() const {
        return soundName;
    }

    void itemOpennessChanged(bool isNowOpen) {
        if ( isNowOpen ) {
            for(int i=0;i<samples->size();i++) {
                addSubItem(new NoteTreeViewItem(i, &soundName, &(samples->getReference(i))));
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

class StatusBar : public juce::Component {
public:
    std::bitset<16> *patternActivity;
    std::bitset<16> *midiActivity;
    juce::String msg;

    void paint (juce::Graphics& g) override {
        g.fillAll(findColour(juce::TextEditor::backgroundColourId));

        int i = 0;
        juce::String display;

        for(int i=0; i<patternActivity->size(); i++) {
            if ( patternActivity->test(i) ) {
                display += "d" + juce::String(i+1) + " " ;
            }
        }

        for(int i=0; i<midiActivity->size(); i++) {
            if ( midiActivity->test(i) ) {
                display += "m" + juce::String(i+1) + " " ;
            }
        }

        patternActivity->reset();
        midiActivity->reset();

        if ( display == "" ) 
            return;

        g.setColour(findColour(juce::Label::textColourId));
        g.drawText(display, 0, 0, getWidth() - 4, getHeight() - 3, juce::Justification::centredRight, true);
        g.drawText(msg, 0, 0, getWidth() - 4, getHeight() - 3, juce::Justification::centredLeft, true);
    }
};

class LogViewer : public juce::TextEditor {
    juce::StringArray *log;
public:
    LogViewer(juce::StringArray *content) {
        log = content;
        setMultiLine(true);
        setReadOnly(true);
        setScrollbarsShown(true);        
    }

    void addPopupMenuItems(juce::PopupMenu &menuToAddTo, const juce::MouseEvent *mouseClickEvent) {
        menuToAddTo.addItem(juce::StandardApplicationCommandIDs::copy, "Copy");
        menuToAddTo.addItem(juce::StandardApplicationCommandIDs::selectAll, "Select All");
        menuToAddTo.addSeparator();
        menuToAddTo.addItem(juce::StandardApplicationCommandIDs::del, "Clear logs");
    }

    void performPopupMenuAction(int menuItemID) {
        if (menuItemID == juce::StandardApplicationCommandIDs::del)
            log->clearQuick();
        else
            juce::TextEditor::performPopupMenuAction(menuItemID);
    }
};


class AliasEditor : public juce::TableListBoxModel {

public:
    AliasEditor(Alias *aliases) {

    }
};
