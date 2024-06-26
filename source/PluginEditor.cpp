/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

class TreeViewSorter {
public:
    static int compareElements(juce::TreeViewItem *i1, juce::TreeViewItem *i2) {
        return i1->getUniqueName().compare(i2->getUniqueName());
    }
};
const TreeViewSorter treeViewSorter;

void NoteTreeViewItem::itemClicked(const juce::MouseEvent &e) {
    if (e.mods.getCurrentModifiers().isRightButtonDown() ) {
        juce::PopupMenu menu;

        menu.addItem("Copy sound name to clipboard", [this] { 
            juce::SystemClipboard::copyTextToClipboard(*(this->sound) + ":" + juce::String(this->idx)); });
        menu.addItem("Open folder", [this] { 
            this->holder->filename.revealToUser(); });
        menu.showMenuAsync(juce::PopupMenu::Options());
    } else {
        DirtAudioProcessorEditor *editor = getOwnerView()->findParentComponentOfClass<DirtAudioProcessorEditor>();
        jassert(editor);        
        editor->playSound(*sound, idx);
    }
}

void RootTreeViewItem::refresh() {
    for(juce::HashMap<juce::String, juce::Array<SampleHolder>>::Iterator i(library->content); i.next();) {
        if ( ! refContent.contains(i.getKey()) ) {
            refContent.set(i.getKey(), 0);
            SoundTreeViewItem *sound = new SoundTreeViewItem(i.getKey(), &(library->content.getReference(i.getKey())));
            addSubItemSorted(treeViewSorter, sound);
        }
    }
}

//==============================================================================
DirtAudioProcessorEditor::DirtAudioProcessorEditor(DirtAudioProcessor &p) : 
    AudioProcessorEditor(&p), audioProcessor(p), soundBrowser("SoundBrowser"), panicButton("Panic"),
       libraryContent("LibraryContent", "Library Content"), logViewer(&(p.logger.content)) {

    menuBar.reset(new juce::MenuBarComponent(this));
    addAndMakeVisible(menuBar.get());

    rootItem = new RootTreeViewItem(&p.library);

    soundBrowser.setRootItem(rootItem);
    soundBrowser.setRootItemVisible(false);
    addAndMakeVisible(soundBrowser);

    addAndMakeVisible(logViewer);
    addAndMakeVisible(panicButton);
    panicButton.onClick = [this] { this->audioProcessor.panic(); };

    addAndMakeVisible(libraryContent);

    statusBar.midiActivity = &(p.midiActivity);
    statusBar.patternActivity = &(p.patternActivity);
    addAndMakeVisible(statusBar);

    setSize(866, 674);
    startTimer(300);

    if ( ! p.dispatch.isConnected() ) {
        juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                    .withIconType (juce::MessageBoxIconType::WarningIcon)
                                    .withTitle ("Alert Box")
                                    .withMessage (juce::String("Unable to listen TidalCycle for port ") + juce::String(p.DIRT_UDP_PORT))
                                    .withButton ("OK"),
                                nullptr);
    }

}

DirtAudioProcessorEditor::~DirtAudioProcessorEditor() {
    stopTimer();
    soundBrowser.deleteRootItem();
}

juce::PopupMenu DirtAudioProcessorEditor::getMenuForIndex(int topLevelMenuIndex, const juce::String& str) {
    juce::PopupMenu ret;
    switch(topLevelMenuIndex) {
    case 0 :
        ret.addItem(configPath, "Dirt library path...", true, false);
        break;
    case 1: 
        ret.addItem(schedOffset, "Set sheduler offset");
        ret.addItem(forceOrbit0, "Route to orbit 0", true, audioProcessor.forceOrbit0);
        ret.addItem(enableDebug, "Event content debug", true, audioProcessor.debugEvent);

        break;
    }
    return ret;
}

void DirtAudioProcessorEditor::menuItemSelected(int id, int y) {
    switch(id) {
    case configPath:
        setLibraryPath();
        break;
    case forceOrbit0: {
            audioProcessor.forceOrbit0 = !audioProcessor.forceOrbit0;
            juce::PropertiesFile *prop = audioProcessor.appProp.getUserSettings();
            prop->setValue("routeOrbit0", audioProcessor.forceOrbit0);
            prop->saveIfNeeded();
        }
        break;
    case enableDebug:
        audioProcessor.debugEvent = !audioProcessor.debugEvent;
        break;
    case schedOffset:
        {
            modalWindow.reset(new juce::AlertWindow("Scheduler Delay", "", juce::AlertWindow::NoIcon));
            modalWindow->addTextEditor("delay", audioProcessor.rootValueTree.getProperty(IDs::scheduleOffset), "Scheduler delay in milliseconds");
            modalWindow->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey, 0, 0));
            modalWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey, 0, 0));

            modalWindow->enterModalState(true, juce::ModalCallbackFunction::create([this](int r) {
                if (r) {
                    int delay = this->modalWindow->getTextEditorContents("delay").getIntValue();
                    audioProcessor.rootValueTree.setProperty(IDs::scheduleOffset, delay, nullptr);
                }
                modalWindow.release();
            }), true);
        }
        break;
    }
}

void DirtAudioProcessorEditor::timerCallback() {
    if ( rootItem->refContent.size() != audioProcessor.library.content.size() )
        rootItem->refresh();
    
    if ( logLines != audioProcessor.logger.content.size() ) {
        logLines = audioProcessor.logger.content.size();
        logViewer.setText(audioProcessor.logger.content.joinIntoString(""));
        logViewer.moveCaretToEnd();
    }

    statusBar.repaint();
}

void DirtAudioProcessorEditor::resized() {
    int width = getWidth();
    int height = getHeight();

    int menuSize = juce::LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight();
    int belowLog =  0.37 * height;

    menuBar->setBounds(0, 0, getWidth(), menuSize);
    panicButton.setBounds(width-55, 5 + menuSize, 50, 25);
    libraryContent.setBounds(5, 3 + menuSize, 295, 25);
    soundBrowser.setBounds(5, 35 + menuSize, 295, height - belowLog - 40 - menuSize);
    logViewer.setBounds(5, height - belowLog, width - 10, belowLog - 27);
    statusBar.setBounds(5, height - 27, width - 10, 25);
}

void DirtAudioProcessorEditor::setLibraryPath() {
    juce::PropertiesFile *prop = audioProcessor.appProp.getUserSettings();
    juce::String samplePath = prop->getValue("samplePath", "");

    modalWindow.reset(new juce::AlertWindow("Dirt Library", "", juce::AlertWindow::NoIcon));
    modalWindow->addTextEditor("path", audioProcessor.library.getSamplePath(), "Sample paths (seperator path ':')");
    modalWindow->addComboBox("lazy", { "Enable", "Disable" }, "Sample lazy load");
    modalWindow->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey, 0, 0));
    modalWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey, 0, 0));

    modalWindow->enterModalState(true, juce::ModalCallbackFunction::create([this](int r) {
        if (r) {
            juce::String path = this->modalWindow->getTextEditorContents("path");
            juce::ComboBox *cb = this->modalWindow->getComboBoxComponent("lazy");
            this->audioProcessor.setSamplePath(path, cb->getSelectedId() == 1);
        }
        modalWindow.release();
    }), true);
}

void DirtAudioProcessorEditor::playSound(juce::String soundName, int n) {
    Event *e = new Event();
    e->time = 0;
    e->sound = soundName;
    e->n = n;

    audioProcessor.library.lookup(soundName, n);
    audioProcessor.dispatch.produce(e);

    //statusBar.msg = "playing: " + audioProcessor.library.getSampleInfo(soundName, n);
}

void DirtAudioProcessorEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}