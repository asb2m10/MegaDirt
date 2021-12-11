#include "Dispatch.h"

//const int PLAY_S = juce::String("s").hashCode();
const juce::StringRef PLAY_S("s");
const juce::StringRef PLAY_ID("_id_");
const juce::StringRef PLAY_CPS("cps");
const juce::StringRef PLAY_CYCLE("cycle");
const juce::StringRef PLAY_DELTA("delta");
const juce::StringRef PLAY_ORBIT("orbit");
const juce::StringRef PLAY_BEGIN("begin");
const juce::StringRef PLAY_END("end");
const juce::StringRef PLAY_NOTE("n");
const juce::StringRef PLAY_MIDICHAN("midichan");
const juce::StringRef PLAY_CCN("ccn");
const juce::StringRef PLAY_CCV("ccv");

juce::String showOSCMessageArgument (const juce::OSCArgument& arg)
    {
        juce::String typeAsString;
        juce::String valueAsString;

        if (arg.isFloat32()) {
            return juce::String(arg.getFloat32()) + "F ";
        } else if (arg.isInt32()) {
            return juce::String(arg.getInt32()) + "I ";
        } else if (arg.isString() + " ") {
            return arg.getString() + " ";
        } else if (arg.isBlob()) {
            printf("fuck blob\n");
            typeAsString = "blob";
            auto& blob = arg.getBlob();
            //valueAsString = String::fromUTF8 ((const char*) blob.getData(), (int) blob.getSize());
        } else {
            typeAsString = "(unknown)";
        }

  
        //oscLogList.add (getIndentationString (level + 1) + "- " + typeAsString.paddedRight(' ', 12) + valueAsString);*/
        return juce::String("");
}


Dispatch::Dispatch() {
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
    oscReceiver.connect(57120);
}

void Dispatch::oscMessageReceived(const juce::OSCMessage& message) {
    printf("%s\n", message.getAddressPattern().toString().toRawUTF8());
    
    // juce::String st;
    // for (auto& arg : message) {
    //     st += showOSCMessageArgument(arg);
    // }
    // printf("%s\n", st.toRawUTF8());
    //oscLogListBox.addOSCMessage (message);
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

    for(int i=0;i<message.size();i+=2) {
        
        if ( ! message[i].isString() ) {
            printf("Wrong message format\n");
            break;
        }

        juce::String item = message[i].getString();

        if ( item == PLAY_S ) {
            event->sound = message[i+1].getString();
        } else if ( item == PLAY_ORBIT ) {
            event->orbit = message[i+1].getInt32();
        } else if ( item == PLAY_BEGIN ) {
            event->begin = message[i+1].getFloat32();
        } else if ( item == PLAY_END ) {
            event->end = message[i+1].getFloat32();
        } else if ( item == PLAY_NOTE ) {
            event->note = message[i+1].getFloat32();
        } else if ( item == PLAY_ORBIT ) {
            event->orbit = message[i+1].getInt32();
        } else if ( item == PLAY_MIDICHAN ) {
            event->midichan = message[i+1].getFloat32();
        } else if ( item == PLAY_CPS ) {
	        event->cps = message[i+1].getFloat32();
	    } else if ( item == PLAY_CYCLE ) {
    	    event->cycle = message[i+1].getFloat32();
	    } else if ( item == PLAY_DELTA ) {
            event->delta = message[i+1].getFloat32();
        }
    }

    juce::String st;
    for (auto& arg : message) {
        st += showOSCMessageArgument(arg);
    }
    printf("%s\n", st.toRawUTF8());

    queue.produce(event);

    //printf("%f %f\n", event->begin, event->end);
}

void Dispatch::oscBundleReceived (const juce::OSCBundle& bundle) {
    juce::OSCTimeTag timeTag = bundle.getTimeTag();
    juce::OSCAddressPattern play("/dirt/play");

    for (auto& element : bundle) {
        if (element.isMessage()) {
            const juce::OSCMessage& message = element.getMessage();

            if ( message.getAddressPattern() == play ) {
                processPlay(message);
            }

            /*oscMessageReceived(element.getMessage());
            if (! element.getMessage().isEmpty() ) {
                printf("\t %s \n", st.toRawUTF8());                    
            }*/
        } else if (element.isBundle()) {
            printf("Caline un bundle dans bundle\n");
        }
    }
}

