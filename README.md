![MegaDirt](assets/logo.png)

MegaDirt is a re-implementation of [SuperDirt](https://github.com/musikinformatik/SuperDirt) for [TidalCycles](https://github.com/tidalcycles/tidal). It is meant to be run into a DAW as a plugin (VST3) for those who wants a minimal sampler with midi output.

To be fair, SuperDirt is a departure from the "classical sampler" where everything is attached to a midi event. I think this is a huge asset and should be explored more. You can use [TidalCycles](https://tidalcycles.org) to trigger samples into different ways since MegaDirt uses the same OSC protocol.

* MegaDirt is a proof of concept, use at your own risk. It was built to see how fast can SuperDirt be reimplemented in C++ with JUCE to be used as a plugin. It expects to use the [Dirt-Sample](https://github.com/tidalcycles/Dirt-Samples) downloaded from SuperDirt quark directory.

### MegaDirt might have those advantage over SuperDirt on Supercolider
* Easier installation. MegaDirt can also be run as a standalone application (great for development)
* Easier audio routing from DAW (orbits are plugin buses) and use DAW audio plugins
* Easier midi routing from DAW and use DAW midi plugins
* TidalCycle events can be in sync with the DAW timeline (in progress)

## Things to know

* Orbits are output buses of the plugin. We are limited to 4 bus (orbits). Those might have to be enable the extra ones from the DAW.
* Renoise doesn't seems to render plugin if the editor window is closed
* Tidal cyles can be sync with DAW playhead, in order to work, you have to set the same tempo/cps on both side

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
- [x] Sync DAW playhead with tidal cycles
- [ ] Windows Dirt sample directory

### SuperDirt effects

Basic SuperDirt effects are now supported. 

- [x] Reverb - # room # size
- [x] Filters - # cutoff # hcutoff # bandf (and the resonance counterpart) #djf 
- [ ] Delay - #delay #delayt #delayfb
- [ ] Crush - #crush
- [ ] Phaser - #phasr #phasdp

### Bulding

Building MegaDirt should be straight forward. The building process is supposed to put the VST3 directory into the standard location.

    git clone --recursive https://github.com/asb2m10/MegaDirt.git
    cd MegaDirt
    mkdir build
    cd build
    cmake ..       # add `-G Xcode` if you want to use Apple Xcode and build it afterwards
    make
