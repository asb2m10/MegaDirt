#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>

//==============================================================================
// Represents the constant parts of an audio sample: its name, sample rate,
// length, and the audio sample data itself.
// Samples might be pretty big, so we'll keep shared_ptrs to them most of the
// time, to reduce duplication and copying.
class Sample final {
public:
    Sample(juce::AudioFormatReader &source, double maxSampleLengthSecs)
            : sourceSampleRate(source.sampleRate), length(juce::jmin(int(source.lengthInSamples),
              int(maxSampleLengthSecs * sourceSampleRate))), data(juce::jmin(2, int(source.numChannels)), length + 4) {
        if (length == 0)
            throw std::runtime_error("Unable to load sample");

        source.read(&data, 0, length + 4, 0, true, true);
    }

    double getSampleRate() const { return sourceSampleRate; }
    int getLength() const { return length; }
    const juce::AudioBuffer<float> &getBuffer() const { return data; }

private:
    double sourceSampleRate;
    int length;
    juce::AudioBuffer<float> data;
};

struct SampleHolder {
    juce::File filename;
    std::shared_ptr<Sample> sample;
};

class Library {
public:
    int getNumSamples() {
        return numSamples;
    }

    int getNumSounds() {
        return numSounds;
    }

    void findContent(juce::String samplePath);
    bool lookup(juce::String name, int note);
    Sample *get(juce::String name, int note);
    juce::Array<SampleHolder> &get(juce::String sound) {
        return content.getReference(sound);
    }

private:
    int numSamples = 0;
    int numSounds = 0;

    juce::HashMap<juce::String, juce::Array<SampleHolder>> content;
};