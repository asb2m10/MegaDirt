# MegaDirt
MegaDirt is a re-implementation of [SuperDirt](https://github.com/musikinformatik/SuperDirt) for [TidalCycles](https://github.com/tidalcycles/tidal). It is meant to be run into a DAW as a plugin.

To be fair, SuperDirt is a departure from the "classical sampler" where everything is attached to a midi event. I think this is a huge asset and should be explored more. You can use [TidalCycles](https://github.com/tidalcycles/tidal) to trigger samples into different ways since MegaDirt uses the same OSC protocol.

*MegaDirt is a proof of concept, use at your own risk. It was built to see how fast can SuperDirt be reimplemented in C++ with JUCE to be used as a plugin. It expects to use the [Dirt-Sample](https://github.com/tidalcycles/Dirt-Samples) downloaded from SuperDirt quark directory. *

MegaDirt might have those advantage over SuperDirt on Supercolider:
* Easier installation. MegaDirt can also be run as a standalone application.
* Easier audio routing from DAW and use DAW audio plugins
* Easier midi routing from DAW and use DAW midi plugins
* TidalCycle events can be in sync with the DAW timeline (in progress)

### What is implemented from SuperDirt

- [x] osc msg midichan / s "midi" - send midi event from DAW
- [x] osc msg gain - sample gain
- [x] osc msg begin - begin of sample to use slices
- [x] osc msg end - end of sample to use slices
- [x] osc msg legato - event length based on event window
- [ ] osc msg sustain - event absolute time
- [ ] osc msg pan - sample pan
- [ ] osc msg ccn / ccv - midi controller change
- [ ] Lazy loading of sampling file
- [ ] Configurable sample path
- [ ] Sync DAW playhead with tidal cycles.
- [ ] Reconfigure OSC timing latency based on Tidal target values

### Bulding

Building MegaDirt should be straight forward. The building process is supposed to put the VST3 directory into the standard location.

    mkdir build
    cd build
    cmake ..       # add `-G Xcode` if you want to use Xcode and build it afterwards
    make