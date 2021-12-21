#include "Library.h"

Library::Library() : juce::Thread("LibraryFinder") {
}

Sample *Library::get(juce::String name, int note) {
    if ( ! content.contains(name) )
        return nullptr;

    if ( content[name].size() < note )
        return nullptr;

    return content[name][note].sample.get();
}

void Library::findContent(juce::String samplePath) {
    soundPaths = juce::StringArray::fromTokens(samplePath, ":", "");
    // restart indexer
    if ( stopThread(200) )
        startThread();
}

void Library::run() {
    juce::AudioFormatManager manager;
    manager.registerBasicFormats();

    for(auto root : soundPaths) {
        juce::File file(root);
        if (! file.isDirectory()) {
            printf("Path %s doesn't exists\n", file.getFullPathName().toRawUTF8());
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

                juce::AudioFormatReader *reader = manager.createReaderFor(soundList[i]);
                if ( reader != nullptr ) {
                    numSamples++;
                    holder.sample.reset(new Sample(*reader, 120));
                    delete reader;
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

/*bool Library::lookup(juce::String name, int note) {
    return content.contains(hashName(name));
}*/
