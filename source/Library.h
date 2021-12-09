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
      : sourceSampleRate(source.sampleRate),
        length(juce::jmin(int(source.lengthInSamples),
                    int(maxSampleLengthSecs * sourceSampleRate))),
        data(juce::jmin(2, int(source.numChannels)), length + 4) {
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

class Library {
public:

    void findContent(juce::String samplePath);
    bool lookup(juce::String name);
    Sample *get(juce::String name);

private:
   juce::HashMap<juce::String, std::shared_ptr<Sample>> content;
};
