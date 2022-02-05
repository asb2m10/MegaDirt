/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <stdarg.h>

const juce::StringRef SOUND_MIDI("midi");

class EventSorter {
public:
    static int compareElements(Event *i1, Event *i2) {
        return (i1->time < i2->time) ? -1 : ((i2->time < i1->time) ? 1 : 0);
    }
};
const EventSorter eventSorter;

void DirtLogger::printf(const char *fmt, ...) {
    char output[4096];
    va_list argptr;
    va_start(argptr, fmt);
    vsnprintf(output, 4095, fmt, argptr);
    va_end(argptr);
    writeToLog(output);
}

//==============================================================================
DirtAudioProcessor::DirtAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
              .withOutput("Orbit0", juce::AudioChannelSet::stereo(), true)
              .withOutput("Orbit1", juce::AudioChannelSet::stereo(), false)
              .withOutput("Orbit2", juce::AudioChannelSet::stereo(), false)
              .withOutput("Orbit3", juce::AudioChannelSet::stereo(), false)),
        dispatch(&library)
              
        // 
#endif
{
    addParameter(gain = new juce::AudioParameterFloat("gain", // parameterID
                                                    "Gain", // parameter name
                                                    0.0f,   // minimum value
                                                    1.0f,   // maximum value
                                                    0.5f)); // default value

    juce::Logger::setCurrentLogger(&logger);

    dispatch.connect(DIRT_UDP_PORT);

    if ( dispatch.isConnected() ) {
        logger.printf("*** MegaDirt listening to Tidal messages on port: %i", DIRT_UDP_PORT);
    } else {
        logger.printf("ERROR: unable to listen to UDP port: %i", DIRT_UDP_PORT);
    }

    juce::PropertiesFile::Options options;
    options.applicationName = "MegaDirt";
    options.osxLibrarySubFolder = "Application Support";
    options.folderName = "MegaDirt";
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
        logger.printf("Trying default path for Dirt-Sample: %s", samplePath.toRawUTF8());
    }

    bool lazyLoading = prop->getBoolValue("lazyLoad", true);
    library.setLazyLoading(lazyLoading);
    library.findContent(samplePath);

    // isActive = false;
}

DirtAudioProcessor::~DirtAudioProcessor() {
    library.shutdown();
    dispatch.flushEvent();
    for(auto event: pendingEv) {
        free(event);
    }
    juce::Logger::setCurrentLogger(nullptr);
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
  dispatch.flushEvent();

  isActive = true;
}

void DirtAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.

  sampler.panic();
  isActive = false;
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DirtAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    return true;

  // FIX THIS, (see how it works with auval)
  // return true;
}
#endif

void DirtAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int numSample = buffer.getNumSamples();
    double currentTm = juce::Time::currentTimeMillis();

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
        //printf("%fn", posInfo.ppqPosition);
        // posInfo.timeInSeconds;
    }

    // get the event from the network thread
    for(Event *e = dispatch.consume(); e != nullptr; e = dispatch.consume()) {
        if ( e->time != 0 && e->time + 500 < currentTm ) {
            logger.printf("Flushing late event from dsp thread. %f %f", e->time, currentTm);
            free(e);
        } else {
            if ( debugEvent ) {
                logger.printf("time:%.0f s:%s cps:%f cycle:%g note:%g n:%g delta:%g legato:%g midichan:%d being:%g end:%g speed:%g unit:%c",
                    e->time, e->sound.toRawUTF8(), e->cps, e->cycle, e->note, e->n, e->delta, e->legato, e->midichan,
                    e->begin, e->end, e->speed, e->unit);
            }
            pendingEv.addSorted(eventSorter, e);
        }
    }

    juce::Array<Event *> noteOff;

    // identify event to be played
    int executeEvent = 0;
    double tmEnd = currentTm + (numSample * 1000 / sampler.sampleRate);
    for(auto event: pendingEv) {
        if ( event->time >= tmEnd )
            break;

        executeEvent++;

        // start of event in sample
        int offsetStart = juce::jmax((double) 0, event->time - currentTm) * (sampler.sampleRate * 0.001);

        // event duration in seconds
        double playLength = (event->sustain != 0 ? event->sustain : event->delta * event->legato);

        if ( event->ccv != -1 && event->ccn != -1 ) {
            midiMessages.addEvent(juce::MidiMessage(0xb0+event->midichan, event->ccn, event->ccv), offsetStart);
            midiActivity.set(event->midichan, true);              
        }

        if ( event->sound == SOUND_MIDI ) {
            int targetNote = (event->note != 0 ? event->note : event->n) + 48;
            midiMessages.addEvent(juce::MidiMessage(0x90+event->midichan, targetNote, event->velocity), offsetStart);              

            if ( event->velocity != 0 ) {
                //logger.printf("%f on  %i %x %f %i\n", currentTm, targetNote, event, event->time, offsetStart);
                event->time += (playLength * 1000) - 1;
                event->velocity = 0;
                midiActivity.set(event->midichan, true);              
                noteOff.add(event);
                continue;
            } else {
                //logger.printf("%f off %i %x %f %i\n", currentTm, targetNote, event, event->time, offsetStart);
            }
        } else {
            Sample *sample = library.get(event->sound, event->n);
            if ( sample != nullptr ) {
                if ( forceObrit0 ) 
                    event->orbit = 0;

                if ( event->orbit * 2 >= totalNumOutputChannels ) {
                    logger.printf("Orbit %d not set in DAW. See plugin output bus.", event->orbit);
                    event->orbit = 0;
                }

                sampler.play(event, sample, offsetStart, playLength * sampler.sampleRate);
            } else {
                logger.printf("Sample %s:%i not found", event->sound.toRawUTF8(), (int) event->note);
            }
            patternActivity.set(event->id, true);
        }

        free(event);
    }

    pendingEv.removeRange(0, executeEvent);

    // reinsert note off
    for(auto e: noteOff) {
        pendingEv.addSorted(eventSorter, e);
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

void DirtAudioProcessor::setSamplePath(juce::String paths, bool lazyLoading) {
    juce::PropertiesFile *prop = appProp.getUserSettings();
    prop->setValue("samplePath", paths);
    prop->setValue("lazyLoad", lazyLoading);
    appProp.saveIfNeeded();
    library.setLazyLoading(lazyLoading);
    library.findContent(paths);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new DirtAudioProcessor();
}

/*

bool DirtAudioProcessor::canApplyBusCountChange (bool isInput, bool isAddingBuses, juce::AudioProcessor::BusProperties& outNewBusProperties) {
    //logger.printf("bus added %s", outNewBusProperties.busName.toRawUTF8());
    return true;
}

void DirtAudioProcessor::numChannelsChanged() {
    //logger.printf("num channel %d", getTotalNumOutputChannels());
}

void DirtAudioProcessor::numBusesChanged() {
    //logger.printf("num bus: %d", getTotalNumOutputChannels());
}

void DirtAudioProcessor::processorLayoutsChanged() {
    //logger.printf("processor layour change %d", getTotalNumOutputChannels());
}

bool DirtAudioProcessor::canAddBus(bool isInput) const {
    return true;
}

*/
