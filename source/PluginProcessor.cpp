/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::StringRef SOUND_MIDI("midi");

//==============================================================================
DirtAudioProcessor::DirtAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
              .withOutput("Out-3-4", juce::AudioChannelSet::stereo(), false)
              .withOutput("Out-5-6", juce::AudioChannelSet::stereo(), false)
              .withOutput("Out-7-8", juce::AudioChannelSet::stereo(), false))
#endif
{
  addParameter(gain = new juce::AudioParameterFloat("gain", // parameterID
                                                    "Gain", // parameter name
                                                    0.0f,   // minimum value
                                                    1.0f,   // maximum value
                                                    0.5f)); // default value
    dispatch.connect(57120);

    juce::PropertiesFile::Options options;
    options.applicationName = "dirty";
    options.osxLibrarySubFolder = "Application Support";
    options.folderName = "dirty";
    options.filenameSuffix = "settings";
    appProp.setStorageParameters(options);

    juce::PropertiesFile *prop = appProp.getUserSettings();
    juce::String samplePath = prop->getValue("samplePath", "");

    if ( samplePath == "" ) {
        juce::File home("~");
        if ((juce::SystemStats::getOperatingSystemType() & juce::SystemStats::MacOSX) != 0) {
            samplePath = home.getChildFile("Library/Application Support/SuperCollider/downloaded-quarks/Dirt-Samples").getFullPathName();
        } else {
            // Linux
            samplePath = home.getChildFile(".local/share/SuperCollider/downloaded-quarks/Dirt-Samples").getFullPathName();
        }

        printf("Trying default path for Dirt-Sample: %s\n", samplePath.toRawUTF8());
    }

    library.findContent(samplePath);
}

DirtAudioProcessor::~DirtAudioProcessor() {
    library.shutdown();
}

//==============================================================================
const juce::String DirtAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool DirtAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool DirtAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool DirtAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double DirtAudioProcessor::getTailLengthSeconds() const {
  return 0.0;
}

int DirtAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int DirtAudioProcessor::getCurrentProgram() { return 0; }

void DirtAudioProcessor::setCurrentProgram(int index) {}

const juce::String DirtAudioProcessor::getProgramName(int index) {
  return {};
}

void DirtAudioProcessor::changeProgramName(int index, const juce::String &newName) {}

//==============================================================================
void DirtAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  sampler.setSampleRate(sampleRate);
}

void DirtAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DirtAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
  // FIX THIS, (see how it works with auval)
  return true;
}
#endif

void DirtAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int numSample = buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    auto *playhead = getPlayHead();
    if (playhead != NULL) {
        juce::AudioPlayHead::CurrentPositionInfo posInfo;
        playhead->getCurrentPosition(posInfo);
        printf("%fn", posInfo.ppqPosition);
        // posInfo.timeInSeconds;
    }

    Event *event = dispatch.consume();
    while(event != nullptr) {
        int offsetStart;
        int playLength = sampler.offset(offsetStart, event);

        if ( event->orbit > orbits.size() ) {
            printf("Orbit value to high %d\n", event->orbit);
            continue;
        }

        orbitActivity.set(event->orbit, true);

        if ( event->sound == SOUND_MIDI ) {
            int targetNote = event->note + 64;
            midiMessages.addEvent(juce::MidiMessage(0x90+event->midichan, targetNote, DEFAULT_MIDI_VELOCITY), offsetStart);
            midiMessages.addEvent(juce::MidiMessage(0x80+event->midichan, targetNote), playLength);
            midiActivity.set(event->midichan, true);
        } else {
            Sample *sample = library.get(event->sound, event->note);
            if ( sample != nullptr ) {
                sampler.play(event, sample, offsetStart, playLength);
            } else {
                printf("Sample %s:%i not found\n", event->sound.toRawUTF8(), (int) event->note);
            }
        }
        free(event);
        event = dispatch.consume();
    }

    sampler.processBlock(buffer, numSample);
    buffer.applyGain(*gain);
}

//==============================================================================
bool DirtAudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *DirtAudioProcessor::createEditor() {
  return new DirtAudioProcessorEditor(*this);
}

//==============================================================================
void DirtAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void DirtAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new DirtAudioProcessor();
}
