#!/bin/sh

g++ -g -O0 -o dtmf -I ../include dtmf.cc ../src/Nco.cc ../src/PhaseAccumulator.cc

g++ -g -O0 -o dialer -I ../include dialer.cc ../src/PcmWriter.cc ../src/Nco.cc ../src/PhaseAccumulator.cc

g++ -g -O0 -o testMajik -I ../include testMajik.cc ../src/MajikWhistle.cc ../src/Nco.cc ../src/PcmWriter.cc ../src/PhaseAccumulator.cc
