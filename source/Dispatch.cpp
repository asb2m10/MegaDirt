#include "Dispatch.h"

const juce::OSCAddressPattern PLAY_PATTERN("/dirt/play");

int note2int(juce::String note) {
    const char *ref = note.toLowerCase().toRawUTF8();
    int ret = 0;

    switch(ref[0]) {
        case 'c' : case 'd' : case 'e' : case 'f' : case 'g' :
            ret = ref[0] - 'c';
        break;
        case 'a' : case 'b' :
            ret = ref[0] - 'a' - 2;
        break;
        default:
            juce::Logger::writeToLog("Invalid note reference: " + note);
            return 0;
    }

    for(int i=1;ref[i] != 0; i++) {
        if ( isdigit(ref[i]) ) {
            int target = ref[i] - '0';
            return ret - (target * 12) - 60;
        }

        switch(ref[i]) {
            case 's' : case '#' :
                ret++;
            break;

            case 'f' :
                ret--;
            break;

            default:
                juce::Logger::writeToLog("Invalid note reference: " + note);
                return ret;            
        }
    }

    return ret;
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

void Dispatch::processPlay(const juce::OSCMessage& message, double time) {
    if ( message.size() % 2 ) {
        juce::Logger::writeToLog("Wrong OSC message size");
        return;
    }

    Event *event = new Event();
    event->serialId = ++serialId;
    event->time = time;
    for(int i=0;i<message.size();i+=2) {
        if ( ! message[i].isString() ) {
            juce::Logger::writeToLog("Wrong OSC message format");
            continue;
        }

        juce::String key = message[i].getString();
        juce::OSCArgument value = message[i+1];

        if ( key == juce::StringRef("_id_") ) {
            event->id = value.getString().getIntValue() - 1;
        } else if ( key == juce::String("cps") ) {
            event->cps = value.getFloat32();
        } else if ( key == juce::String("cycle") ) {
            event->cycle = value.getFloat32();
        } else if ( key == juce::String("delta") ) {
            event->delta = value.getFloat32();
        } else if ( key == juce::String("s") ) {
            event->sound = value.getString();
        } else if ( key == juce::String("n") ) {
            event->n = value.isFloat32() ? value.getFloat32() : value.getInt32();
        } else if ( key == juce::String("note") ) {
            if ( value.isString() )
                event->note = note2int(value.getString());
            else
                event->note = value.isFloat32() ? value.getFloat32() : value.getInt32();
        } else if ( key == juce::String("orbit") ) {
            event->orbit = value.getInt32();
        } else if ( key == juce::String("unit") ) {
            event->unit = value.getString()[0];
        } else if ( key == juce::String("velocity") ) {
            event->velocity = value.getFloat32();
        } else {
            if ( ! value.isFloat32() ) {
                juce::Logger::writeToLog(juce::String("Key not mapped: ") + key);
            } else {
                event->keys.set(key, value.getFloat32());
            }
        }
    }
    
    if ( event->sound != juce::StringRef("midi") ) {
        if ( ! library->lookup(event->sound, event->n) ) {
            juce::Logger::writeToLog(juce::String("Sound not found: ") + event->sound);
            free(event);
            return;
        }
    } 

    queue.produce(event);
}

void Dispatch::oscBundleReceived(const juce::OSCBundle& bundle) {
    double timeTag = bundle.getTimeTag().toTime().toMilliseconds();

    //printf("Delta %i\n", timeTag.toTime().toMilliseconds() - juce::Time::getCurrentTime().toMilliseconds());

    if ( timeTag < juce::Time::getCurrentTime().toMilliseconds() ) {
        juce::Logger::writeToLog("Warning: ignoring event in the past");
        return;
    }

    for (auto& element : bundle) {
        if (element.isMessage()) {
            const juce::OSCMessage& message = element.getMessage();
            if ( message.getAddressPattern() == PLAY_PATTERN ) {
                processPlay(message, timeTag);
                continue;
            }
        }
    }
}
