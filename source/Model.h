#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace IDs
{
#define DECLARE_ID(name) const juce::Identifier name (#name);
    DECLARE_ID(ROOT)
    DECLARE_ID(eventDebug)
    DECLARE_ID(forceOrbit)
    DECLARE_ID(scheduleOffset)
}
