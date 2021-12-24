#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "juce_osc/juce_osc.h"
#include "LockFreeQueue.h"

struct Event {
    juce::String sound;

    // id
    int orbit = 0;

    // timing **
    float cps = 0;              // cycle-per-seconds (tempo)
    float cycle = 0;            // current event cycles
    float delta = 0;            // item duration in seconds (based on cycle)

    // note / pitch
    float note = 0;             // item note, if sample will be pitched
    float n = 0;                // item sample index in sound
    char unit = ' ';            // unit for the pitch

    float begin = 0;            // begin to play on sample (in %)
    float end = 1;              // end to play on sample (in %)

    // event timing
    float legato = 1;           // event duration based on delta
    float sustain = 0;          // absolute event time (superseed delta) in seconds

    // midi
    int midichan = 0;           // midi channel for event
    float ccn;                  // midi controller value
    float ccv;                  // midi controller value
};


class Dispatch : private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback> {
public:
    Dispatch();
    ~Dispatch();

    void connect(int port);
    void oscMessageReceived(const juce::OSCMessage& message) override;
    void oscBundleReceived (const juce::OSCBundle& bundle) override;

    /**
     * @brief Consume event generated from OSC thread to DSP thread
     */
    Event *consume();

    /** 
     * Insert event in DSP queue
     */
    void produce(Event *event) {
        queue.produce(event);
    }

    /**
     * @brief Return true if the OSC is listening to UDP port
     */
    bool isConnected() {
        return connected;
    }
private:
    bool connected = false;

    void processPlay(const juce::OSCMessage& message);
    juce::LockFreeQueue<Event *> queue;
    juce::OSCReceiver oscReceiver;
};
