/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

void NoteTreeViewItem::itemClicked(const juce::MouseEvent &e) {
    DirtAudioProcessorEditor *editor = getOwnerView()->findParentComponentOfClass<DirtAudioProcessorEditor>();

    jassert(editor);

/*    TreeViewItem *sound = getParentItem();
    TreeViewItem *rootGeneric = sound->getParentItem();
    RootTreeViewItem *root = dynamic_cast<RootTreeViewItem *>(rootGeneric);

    root->
    */
    printf("click %s\n", sound->toRawUTF8());
}

//==============================================================================
DirtAudioProcessorEditor::DirtAudioProcessorEditor(DirtAudioProcessor &p) : 
    AudioProcessorEditor(&p), audioProcessor(p), soundBrowser("SoundBrowser"), panicButton("Panic"), showLog("showLog") {

    rootItem = new RootTreeViewItem(&p.library);

    soundBrowser.setRootItem(rootItem);
    soundBrowser.setRootItemVisible(false);
    addAndMakeVisible(soundBrowser);


    showLog.setMultiLine(true);
    showLog.setReadOnly(true);
    showLog.setScrollbarsShown(true);

    addAndMakeVisible(showLog);
    addAndMakeVisible(panicButton);

    setSize(700, 400);
    startTimer(100);

}

DirtAudioProcessorEditor::~DirtAudioProcessorEditor() {
//    soundBrowser.deleteAllChildren();
    stopTimer();
}

void DirtAudioProcessorEditor::timerCallback() {
    if ( rootItem->refContent.size() != audioProcessor.library.content.size() )
      rootItem->refresh();
}

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
    soundBrowser.setBounds(5, 5, 300, height - 5);
    showLog.setBounds(305, (height - 10) / 2, width - 305, (height - 10) / 2);
}
