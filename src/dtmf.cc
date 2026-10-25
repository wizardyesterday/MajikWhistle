//*************************************************************************
// File name: cosine.cc
//*************************************************************************

//*************************************************************************
// This program generates DTMF tones that are suitable for dialers on
// the old PSTN. Two numerically oscillators are used for tone generation,
// and if only one oscillator needs to be used, the frequency and
// amplitude of that oscillator will be used. The program was written in
// order to wring out any bugs before it is integrated into the
// MajikWhistle touch tone generator program.
// The NCO data is written to an output file with the format described
// below.  Note that only the in-phase component of the NCO output is
// used, therefore the output file represents a cosine waveform.
//
// To run this program type,
// 
//     ./dtmf -a amplitude1 -A amplitude2 -f frequency1 -F frequency2
//            -r sampleRate -d duration (ms) > ncoFileName,
//
// where,
//
//    amplitude1 - The ampoitude between 0 and 1 inclusive, of NC0 1.
//    amplitude2 - The ampoitude between 0 and 1 inclusive, of NC0 2.
//    frequency1 - frequency in Hz of NCO 1.
//    frequency2 - frequency in Hz of NCO 2.
//    sampleRate - The sample rate in samples/second.
//    duration - The duration in milliseconds.
//*************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "Nco.h"

// This structure is used to consolidate user parameters.
struct MyParameters
{
  float *amplitude1Ptr;
  float *amplitude2Ptr;
  float *frequency1Ptr;
  float *frequency2Ptr;
  float *sampleRatePtr;
  float *durationPtr;
};

/*****************************************************************************

  Name: getUserArguments

  Purpose: The purpose of this function is to retrieve the user arguments
  that were passed to the program.  Any arguments that are specified are
  set to reasonable default values.

  Calling Sequence: exitProgram = getUserArguments(parameters)

  Inputs:

    parameters - A structure that contains pointers to the user parameters.

  Outputs:

    exitProgram - A flag that indicates whether or not the program should
    be exited.  A value of true indicates to exit the program, and a value
    of false indicates that the program should not be exited..

*****************************************************************************/
bool getUserArguments(int argc,char **argv,struct MyParameters parameters)
{
  bool exitProgram;
  bool done;
  int opt;

  // Default not to exit program.
  exitProgram = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default parameters.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default 1/4 scale.
  *parameters.amplitude1Ptr = 0.25;

  // Default to zero amplitude.
  *parameters.amplitude2Ptr = 0;

  // Default to 200Hz.
  *parameters.frequency1Ptr = 200;

  // Default to 0Hz.
  *parameters.frequency2Ptr = 0;

  // Default to 8000 S/s.
  *parameters.sampleRatePtr = 8000;

  // Default for a 1 second signal.
  *parameters.durationPtr = 1;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Set up for loop entry.
  done = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Retrieve the command line arguments.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  while (!done)
  {
    // Retrieve the next option.
    opt = getopt(argc,argv,"a:A:f:F:r:d:h");

    switch (opt)
    {
      case 'a':
      {
        *parameters.amplitude1Ptr = atof(optarg);
        *parameters.amplitude1Ptr = fabs(*parameters.amplitude1Ptr);
        break;
      } // case

      case 'A':
      {
        *parameters.amplitude2Ptr = atof(optarg);
        *parameters.amplitude2Ptr = fabs(*parameters.amplitude2Ptr);
        break;
      } // case

      case 'f':
      {
        *parameters.frequency1Ptr = atof(optarg);
        break;
      } // case

      case 'F':
      {
        *parameters.frequency2Ptr = atof(optarg);
        break;
      } // case

      case 'r':
      {
        *parameters.sampleRatePtr = atof(optarg);
        break;
      } // case

      case 'd':
      {
        *parameters.durationPtr = atof(optarg);
        break;
      } // case

      case 'h':
      {

        // Display usage.
        fprintf(stderr,"./dtmf -a amplitude1 -A amplitude2"
                " -f frequency1 -F frequency2 -r sampleRate"
                "\n       -d duration, ms\n");

        // Indicate that program must be exited.
        exitProgram = true;
        break;
      } // case

      case -1:
      {
        // All options consumed, so bail out.
        done = true;
      } // case
    } // switch

  } // while
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return (exitProgram);

} // getUserArguments

//*************************************************************************
// Mainline code.
//*************************************************************************
int main(int argc,char **argv)
{
  int i;
  bool exitProgram;
  float iValue, qValue;
  int16_t cosineValue;
  int16_t cosineValue1;
  int16_t cosineValue2;
  float amplitude1;
  float amplitude2;
  float frequency1;
  float frequency2;
  float sampleRate;
  float duration;
  int numberOfSamples;
  Nco *myNco1Ptr;
  Nco *myNco2Ptr;
  struct MyParameters parameters;

  // Set up for parameter transmission.
  parameters.amplitude1Ptr = &amplitude1;
  parameters.amplitude2Ptr = &amplitude2;
  parameters.frequency1Ptr = &frequency1;
  parameters.frequency2Ptr = &frequency2;
  parameters.sampleRatePtr = &sampleRate;
  parameters.durationPtr = &duration;

  // Retrieve the system parameters.
  exitProgram = getUserArguments(argc,argv,parameters);

  if (exitProgram)
  {
    // Bail out.
    return (0);
  } // if

  // We derive this converting milliseconds to seconds.
  numberOfSamples = (int)((sampleRate * duration / 1000));

  // Instantiate an NCO 1.
  myNco1Ptr = new Nco(sampleRate,frequency1);

  // Instantiate an NCO 2.
  myNco2Ptr = new Nco(sampleRate,frequency2);

  for (i = 0; i < numberOfSamples; i++)
  {
    // Get the next sample pair.
    myNco1Ptr->runFast(&iValue,&qValue);

    // Convert to integer and scale.
    cosineValue1 = (int16_t)(iValue * amplitude1 * 32767);

    // Get the next sample pair.
    myNco2Ptr->runFast(&iValue,&qValue);

    // Convert to integer and scale.
    cosineValue2 = (int16_t)(iValue * amplitude2 * 32767);

   // Sum the two oscillator values.
   cosineValue = cosineValue1 + cosineValue2;

   // Write the samples to stdout
   fwrite(&cosineValue,sizeof(int16_t),1,stdout);
  } // for

  // Release resource.
  if (myNco1Ptr != NULL)
  {
    delete myNco1Ptr;
  } // if

  // Release resource.
  if (myNco2Ptr != NULL)
  {
    delete myNco2Ptr;
  } // if

  return (0);

} // main
