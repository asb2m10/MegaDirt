![MegaDirt](assets/logo.png)

MegaDirt is a dirty (hence the name MegaDirt) cheap implementation of [SuperDirt](https://github.com/musikinformatik/SuperDirt)
for [TidalCycles](https://tidalcycles.org/), [strudel](https://strudel.cc/) or [Sardine](https://github.com/Bubobubobubobubo/sardine).
It is not meant to replace SuperDirt but offer an alternative to "Live Coding" environment for users that want to use a simple solution
to trigger midi notes / simply trigger samples from a DAW.

MegaDirt is a proof of concept, it was built to see how fast can SuperDirt be reimplemented in C++ with JUCE to be used as a plugin as an MVP.

It expects to use the [Dirt-Sample](https://github.com/tidalcycles/Dirt-Samples) collection
downloaded from SuperDirt quark directory. Otherwise you can customize this directory in the settings menu.
* On Linux it will look for "~/.local/share/SuperCollider/downloaded-quarks/Dirt-Samples"
* On macOS it will look for for "~/Library/Application Support/SuperCollider/downloaded-quarks/Dirt-Samples"
* On Windows it will look for "%localappdata%/SuperCollider/downloaded-quarks/Dirt-Samples"

### MegaDirt might have those advantages over SuperDirt on Supercollider
* Easier installation. MegaDirt can also be run as a standalone application (great for development)
* Easier audio routing from DAW (orbits are plugin buses) and use DAW audio plugins
* Easier midi routing from DAW and use DAW midi plugins (TidalCycles)

## Things to know

* Orbits are output buses of the plugin.
* DAW are picky about absolute time, feel free to set the schedule offset based on the audio interface and plugin chain for accurate timing.

### What is implemented from SuperDirt

- [x] osc msg midichan / s "midi" - send midi event from DAW
- [x] osc msg gain - sample gain
- [x] osc msg begin/end - sample begin/end to use for slices
- [x] osc msg legato - event length based on event window
- [x] osc msg sustain - event absolute time
- [x] osc msg pan - sample pan
- [x] osc msg ccn / ccv - midi controller change
- [x] osc msg orbit - sample output bus
- [x] osc msg speed - sample speed
- [x] osc msg loop - loop the sample within window playback
- [x] Lazy loading of sampling files based on dirt folder
- [x] Configurable sample path
- [x] Character (a5, c, a#, gb) note names
- [x] Multichannel orbit support (based on plugin bus)
- [x] Support loop units ('r' and 'c')
- [X] Windows Dirt sample directory
- [ ] Implement control buses (plug to Tidal)

### Known Issues
- [ ] Find why renoise only render plugin when the window is open

### SuperDirt effects

Basic SuperDirt effects are now supported.

- [x] Reverb - # room # size
- [x] Filters - # cutoff # hcutoff # bandf (and the resonance counterpart) #djf
- [x] Delay - #delay #delayt #delayfb
- [x] Crush - #crush
- [ ] Phaser - #phasr #phasdp

### Bulding

Building MegaDirt should be straight forward. The building process is supposed to put the VST3 directory into the standard location.

    git clone --recursive https://github.com/asb2m10/MegaDirt.git
    cd MegaDirt
    mkdir build
    cd build
    cmake ..       # add `-G Xcode` if you want to use Apple Xcode and build it afterwards
    make
