#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_osc/juce_osc.h>
#include "LockFreeQueue.h"
#include "Library.h"

struct Event {
    juce::String sound;
    double time;               // item time in milliseconds

    // id
    int id = 0;
    int orbit = 0;
    int serialId = 0;

    // timing **
    float cps = 0;              // cycle-per-seconds (tempo)
    float cycle = 0;            // current event cycles

    // note / pitch
    float note = 0;             // item note, if sample will be pitched
    float n = 0;                // item sample index in sound
    char unit = 'r';            // unit for the pitch
    //float speed = 1;

    // sample playback
    //float begin = 0;            // begin to play on sample (in %)
    //float end = 1;              // end to play on sample (in %)
    //float gain = 1;
    //float pan = 0.5;

    // event timing
    float delta = 0;            // item duration in seconds (based on cycle)
    //float legato = 1;           // event duration based on delta
    //float sustain = 0;          // absolute event time (superseed delta) in seconds

    // midi
    //int midichan = 0;           // midi channel for event
    int velocity = 100;
    //float ccn = -1;             // midi controller value
    //float ccv = -1;             // midi controller value

    juce::HashMap<juce::String, float> keys;

    float get(juce::String key, float defaultValue) {
        return keys.contains(key) ? keys[key] : defaultValue;
    }

    float get(juce::String key) {
        return keys[key];
    }

    bool hasKey(juce::String key) {
        return keys.contains(key);
    }

    bool hasKeys(juce::StringArray array) {
        for(auto &i: array) {
            if ( keys.contains(i) )
                return true;
        }
        return false;
    }

    double debug;
};


class Dispatch : private juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback> {
public:
    Dispatch(Library *lib);
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

    /**
     * @brief Flush event if the DSP queue is re-sync
     * 
     */
    void flushEvent() {
        Event *e = consume();
        while(e != nullptr) {
            free(e);
            e = consume();
        }
    }
private:
    bool connected = false;

    void processPlay(const juce::OSCMessage& message, double time);
    juce::LockFreeQueue<Event *> queue;
    juce::OSCReceiver oscReceiver;

    Library *library;

    int serialId = 0;
};
