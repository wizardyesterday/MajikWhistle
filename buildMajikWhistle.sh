#!/bin/sh

g++ -g -O0 -o majikWhistle -I include src/majikWhistle.cc src/diag_ui.cc src/console.cc src/MajikWhistle.cc src/Nco.cc src/PcmWriter.cc src/PhaseAccumulator.cc -lm -lpthread -lrt 

