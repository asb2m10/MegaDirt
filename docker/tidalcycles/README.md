asb2m10/tidalcycles
===================

Basic Haskell image to run [TidalCycles](https://tidalcycles.org/) on the command line. SuperDirt or MegaDirt is desgined to be run locally under this configuration.

Networking on docker with a real host communication can be tricky (e.g. host.docker.internal doesn't always route UDP packets to host), it is best to provide your IP address where Superdirt is running. Unless you run host networking on Linux, 127.0.0.1 will not reach docker host machine.

```
docker run -it --rm asb2m10/tidalcycles <your host that is running Dirt IP>
```
