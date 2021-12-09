#include "Library.h"

juce::String hashName(juce::String name) {
    if ( name.containsChar(':') )
        return name;
    return name + ":1";
}

Sample *Library::get(juce::String name) {
    juce::String fullname = hashName(name);
    if ( ! content.contains(fullname) )
        return nullptr;
    return content[fullname].get();
}

void Library::findContent(juce::String samplePath) {
    juce::StringArray paths = juce::StringArray::fromTokens(samplePath, ":", "");
    juce::Thread::launch([this, paths] {
        juce::AudioFormatManager manager;
        manager.registerBasicFormats();

        for(auto root : paths) {
            juce::File file(root);
            if (! file.isDirectory()) {
                printf("Path %s doesn't exists\n", file.getFullPathName().toRawUTF8());
                continue;
            }

             for(auto soundFile : file.findChildFiles(juce::File::TypesOfFileToFind::findDirectories, false, "*")) {
                juce::Array<juce::File> soundList = soundFile.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false, "*.wav");
                soundList.sort();
                juce::String soundName = soundFile.getFileName();

                for(int i=0; i<soundList.size();i++) {
                   juce::String name = soundName + ":" + juce::String(i+1);
                   juce::AudioFormatReader *reader = manager.createReaderFor(soundList[i]);
                   if ( reader != nullptr ) {
                      std::shared_ptr<Sample> sample;
                      sample.reset(new Sample(*reader, 120));
                      content.set(name, sample);
                      delete reader;
                   }
                }
             }
          }

          printf("Finished reading %d samples\n", content.size());
    });
}

bool Library::lookup(juce::String name) {
    return content.contains(hashName(name));
}
