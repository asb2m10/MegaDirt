#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "juce_osc/juce_osc.h"
#include "LockFreeQueue.h"

struct Event {
    juce::String sound;

    int orbit = 0;
    float begin = 0;
    float end = 1;
    float note = 0;
    float cps;
    float cycle;
    float delta;

    float legato = -1;

    float ccv; 
    float ccn;

    int midichan = 0;
};

class Dispatch : private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback> {
public:
    Dispatch();
    ~Dispatch();

    void connect(int port);
    void oscMessageReceived(const juce::OSCMessage& message) override;
    void oscBundleReceived (const juce::OSCBundle& bundle) override;

    Event *consume();
private:

    void processPlay(const juce::OSCMessage& message);
    juce::LockFreeQueue<Event *> queue;
    juce::OSCReceiver oscReceiver;
};
