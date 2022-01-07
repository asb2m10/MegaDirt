#include "Dispatch.h"

const juce::OSCAddressPattern PLAY_PATTERN("/dirt/play");

int note2int(juce::String note) {
    char first = note.toLowerCase().toRawUTF8()[0] - 99;
    return first;
}

juce::String showOSCMessageArgument (const juce::OSCArgument& arg) {
    juce::String typeAsString;
    juce::String valueAsString;

    if (arg.isFloat32()) {
        return juce::String(arg.getFloat32()) + "F ";
    } else if (arg.isInt32()) {
        return juce::String(arg.getInt32()) + "I ";
    } else if (arg.isString() + " ") {
        return arg.getString() + " ";
    } else if (arg.isBlob()) {
        printf("blob\n");
        typeAsString = "blob";
        auto& blob = arg.getBlob();
        //valueAsString = String::fromUTF8 ((const char*) blob.getData(), (int) blob.getSize());
    } else {
        typeAsString = "(unknown)";
    }
    //oscLogList.add (getIndentationString (level + 1) + "- " + typeAsString.paddedRight(' ', 12) + valueAsString);*/
    return juce::String("");
}


Dispatch::Dispatch(Library *lib) : library(lib)  {
    oscReceiver.registerFormatErrorHandler ([this] (const char* data, int dataSize)
                                {
                                /*  String(dataSize)
                                    oscLogListBox.addInvalidOSCPacket (data, dataSize);*/
                                
                                });
    oscReceiver.addListener(this);

    oscMapper.set("s", P_S);
    oscMapper.set("note", P_NOTE);
    oscMapper.set("n", P_N);
    oscMapper.set("_id_", P_ID);
    oscMapper.set("cps", P_CPS);
    oscMapper.set("cycle", P_CYCLE);
    oscMapper.set("delta", P_DELTA);
    oscMapper.set("midichan", P_MIDICHAN);
    oscMapper.set("ccn", P_CCN);
    oscMapper.set("ccv", P_CCV);
    oscMapper.set("legato", P_LEGATO);
    oscMapper.set("unit", P_UNIT);
    oscMapper.set("octave", P_OCTAVE);
    oscMapper.set("sustain", P_SUSTAIN);
    oscMapper.set("orbit", P_ORBIT);
    oscMapper.set("gain", P_GAIN);
    oscMapper.set("pan", P_PAN);
    oscMapper.set("begin", P_BEGIN);
    oscMapper.set("end", P_END);
    oscMapper.set("speed", P_SPEED);
}

Dispatch::~Dispatch() {

}

void Dispatch::connect(int port) {
    connected = oscReceiver.connect(port);
}

void Dispatch::oscMessageReceived(const juce::OSCMessage& message) {
    //printf("%s\n", message.getAddressPattern().toString().toRawUTF8());
}

Event *Dispatch::consume() {
    Event *event = nullptr;
    queue.consume(event);
    return event;
}

void Dispatch::processPlay(const juce::OSCMessage& message) {
    if ( message.size() % 2 ) {
        printf("Wrong message size\n");
        return;
    }

    Event *event = new Event();
    event->serialId = ++serialId;
    for(int i=0;i<message.size();i+=2) {
        if ( ! message[i].isString() ) {
            printf("Wrong message format\n");
            continue;
        }

        juce::String key = message[i].getString();
        juce::OSCArgument value = message[i+1];

        if ( !oscMapper.contains(key) ) {
            printf("Key %s not mapped\n", key.toRawUTF8());
            continue;
        }

        switch(oscMapper[key]) {
            case P_ID:
                // NOP
            break;

            case P_CPS:
                event->cps = value.getFloat32();
            break;

            case P_CYCLE:
                event->cycle = value.getFloat32();
            break;

            case P_DELTA:
                event->delta = value.getFloat32();
            break;

            case P_S:
                event->sound = value.getString();
            break;

            case P_N:
                event->n = value.isFloat32() ? value.getFloat32() : value.getInt32();
            break;

            case P_NOTE:
                if ( value.isString() ) 
                    event->note = note2int(value.getString());
                else
                    event->note = value.isFloat32() ? value.getFloat32() : value.getInt32();
            break;

            case P_BEGIN:
                event->begin = value.getFloat32();
            break;

            case P_END :
                event->end = value.getFloat32();
            break;

            case P_ORBIT:
                event->orbit = value.getInt32();
            break;

            case P_SPEED: 
                event->speed = value.getFloat32();
            break;

            case P_UNIT:
                event->unit = value.getString()[0];
            break;

            case P_GAIN:
                event->gain = value.getFloat32();
            break;

            case P_PAN:
                event->pan = value.getFloat32();
            break;

            case P_CCN:
                event->ccn = value.getFloat32();
            break;

            case P_CCV:
                event->ccv = value.getFloat32();
            break;

            default:
                printf("Key not configured %s\n", key.toRawUTF8());
        }
    }
    
    if ( event->sound != juce::StringRef("midi") ) {
        if ( ! library->lookup(event->sound, event->n) ) {
            printf("Sound %s not found.\n", event->sound.toRawUTF8() );
            free(event);
            return;
        }
    } 

    queue.produce(event);
}

void Dispatch::oscBundleReceived(const juce::OSCBundle& bundle) {
    juce::OSCTimeTag timeTag = bundle.getTimeTag(); 

    //printf("Delta %i\n", timeTag.toTime().toMilliseconds() - juce::Time::getCurrentTime().toMilliseconds());

    for (auto& element : bundle) {
        if (element.isMessage()) {
            const juce::OSCMessage& message = element.getMessage();
            if ( message.getAddressPattern() == PLAY_PATTERN ) {
                processPlay(message);
                continue;
            }
        } else if (element.isBundle()) {
            printf("Caline un bundle dans bundle\n");
        }
    }
}
