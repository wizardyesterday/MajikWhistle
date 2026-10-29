#!/bin/sh

g++ -g -O0 -o dtmf -I include rapidPrototype/dtmf.cc src/Nco.cc src/PhaseAccumulator.cc

g++ -g -O0 -o dialer -I include rapidPrototype/dialer.cc src/PcmWriter.cc src/Nco.cc src/PhaseAccumulator.cc

g++ -g -O0 -o testMajik -I include rapidPrototype/testMajik.cc src/MajikWhistle.cc src/Nco.cc src/PcmWriter.cc src/PhaseAccumulator.cc
