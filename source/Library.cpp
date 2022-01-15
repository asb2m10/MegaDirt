#include "Library.h"

Library::Library() : juce::Thread("LibraryFinder") {
    manager.registerBasicFormats();
}

Sample *Library::get(juce::String name, int note) {
    if ( ! content.contains(name) )
        return nullptr;

    note = note % content[name].size();
    return content.getReference(name).getReference(note).sample.get();
}

void Library::findContent(juce::String sp) {
    samplePath = sp;
    soundPaths = juce::StringArray::fromTokens(sp, ":", "");
    // restart indexer
    if ( stopThread(200) )
        startThread();
}

void Library::run() {
    for(auto root : soundPaths) {
        juce::File file(root);
        if (! file.isDirectory()) {
            //juce::Logger::writeToLog("Path doesn't exists", file.getFullPathName().toRawUTF8());
            continue;
        }

        for(auto soundFile : file.findChildFiles(juce::File::TypesOfFileToFind::findDirectories, false, "*")) {
            if ( soundFile.getFileName().startsWith(".") )
                continue;

            juce::Array<juce::File> soundList = soundFile.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false, "*.wav");
            soundList.sort();
            juce::Array<SampleHolder> holders;
            for(int i=0; i<soundList.size();i++) {
                SampleHolder holder;
                holder.filename = soundList[i];
                if ( !lazyLoading ) {
                    juce::AudioFormatReader *reader = manager.createReaderFor(soundList[i]);
                    if ( reader != nullptr ) {
                        numSamples++;
                        holder.sample.reset(new Sample(*reader, 120));
                        delete reader;
                    }
                }
                holders.add(holder);
            }
            numSounds++;
            content.set(soundFile.getFileName(), holders);
            if ( threadShouldExit() ) {
                printf("Exiting thread reading...\n");
                return;
            }
        }
    }
    printf("Finished reading %d samples\n", numSamples);
}

bool Library::lookup(juce::String name, int note) {
    if (! content.contains(name))
        return false;

    juce::Array<SampleHolder> &holders = content.getReference(name);

    note = note % holders.size();
    SampleHolder &holder = holders.getReference(note);

    if ( holder.sample.get() == nullptr ) {
        juce::AudioFormatReader *reader = manager.createReaderFor(holder.filename);
        if ( reader != nullptr ) {
            numSamples++;
            holder.sample.reset(new Sample(*reader, 120));
            delete reader;
        }
    }

    return true;
}
