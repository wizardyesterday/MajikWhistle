#!/bin/sh

g++ -g -O0 -o dtmf -I include src/dtmf.cc src/Nco.cc src/PhaseAccumulator.cc
g++ -g -O0 -o dialer -I include src/dialer.cc src/Nco.cc src/PhaseAccumulator.cc

