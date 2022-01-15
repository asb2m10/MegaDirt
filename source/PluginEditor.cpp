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
    DirtAudioProcessorEditor *editor = getOwnerView()->findParentComponentOfClass<DirtAudioProcessorEditor>();
    jassert(editor);
    editor->playSound(*sound, idx);
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
    AudioProcessorEditor(&p), audioProcessor(p), soundBrowser("SoundBrowser"), panicButton("Panic"), showLog("showLog"),
       libraryContent("LibraryContent", "Library Content"), libraryPath("Settings...") {

    rootItem = new RootTreeViewItem(&p.library);

    soundBrowser.setRootItem(rootItem);
    soundBrowser.setRootItemVisible(false);
    addAndMakeVisible(soundBrowser);

    showLog.setMultiLine(true);
    showLog.setReadOnly(true);
    showLog.setScrollbarsShown(true);

    addAndMakeVisible(showLog);
    addAndMakeVisible(panicButton);
    panicButton.onClick = [this] { this->audioProcessor.sampler.panic(); };

    addAndMakeVisible(libraryContent);
    addAndMakeVisible(libraryPath);
    libraryPath.onClick = [this] { this->setLibraryPath(); };
    
    statusBar.midiActivity = &(p.midiActivity);
    statusBar.orbitActivity = &(p.orbitActivity);
    addAndMakeVisible(statusBar);

    setSize(900, 500);
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

void DirtAudioProcessorEditor::timerCallback() {
    if ( rootItem->refContent.size() != audioProcessor.library.content.size() )
        rootItem->refresh();
    
    if ( logLines != audioProcessor.logger.content.size() ) {
        logLines = audioProcessor.logger.content.size();
        showLog.setText(audioProcessor.logger.content.joinIntoString(""));
        showLog.moveCaretToEnd();
    }

    statusBar.repaint();
}

void DirtAudioProcessorEditor::resized() {
    int width = getWidth();
    int height = getHeight();

    panicButton.setBounds(width-55, 5, 50, 25);
    libraryContent.setBounds(5, 3, 295, 25);
    libraryPath.setBounds(175, 3, 120, 25);

    statusBar.setBounds(5, height-30, width - 10, 25);
    soundBrowser.setBounds(5, 35, 295, height - 85);
    showLog.setBounds(305, (height - 20) / 2, width - 315, (height - 60) / 2);
}

void DirtAudioProcessorEditor::setLibraryPath() {
    juce::PropertiesFile *prop = audioProcessor.appProp.getUserSettings();
    juce::String samplePath = prop->getValue("samplePath", "");

    settingsWindow = new juce::AlertWindow("Settings", "", juce::AlertWindow::NoIcon);
    settingsWindow->addTextEditor("path", audioProcessor.library.getSamplePath(), "Sample paths (seperator path ':')");
    settingsWindow->addComboBox("lazy", { "Enable", "Disable" }, "Sample lazy load");
    settingsWindow->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey, 0, 0));
    settingsWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey, 0, 0));

    settingsWindow->enterModalState(true, juce::ModalCallbackFunction::create([this](int r) {
        if (r) {
        }
    }), true);
}

void DirtAudioProcessorEditor::playSound(juce::String soundName, int n) {
    Event *e = new Event();
    e->sound = soundName;
    e->n = n;

    audioProcessor.library.lookup(soundName, n);
    audioProcessor.dispatch.produce(e);
}