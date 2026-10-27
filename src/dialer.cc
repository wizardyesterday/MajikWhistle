//*************************************************************************
// File name: dialer.cc
//*************************************************************************

//*************************************************************************
// This program generates blue box DTMF tones that are suitable for use on
// the old PSTN. Two numerically controlled oscillators are used for tone
// generation,
// The program was written in to wring out any bugs before it is
// integrated into the MajikWhistle touch tone generator program.
// The NCO data is written to /dev/dsp in signed 16-bit little endian
// PCM audio samples.
//
// To run this program type,
// 
//     ./dialer -r sampleRate -d duration (ms)
//
// where,
//
//    sampleRate - The sample rate in samples/second.
//    duration - The duration in milliseconds.
//*************************************************************************

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <termios.h>

#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "Nco.h"

#define SIZE (16)
#define CHANNELS (1)

// This structure is used to consolidate user parameters.
struct MyParameters
{
  float *sampleRatePtr;
  float *durationPtr;
};

int fd;
char textBuffer[200];

// This buffer holds the digitized audio.
int16_t buf[256000];

void init(int sampleRate);
void writePcmSamples(int16_t *buffer,int numberOfSamples);

void setEcho(int enable)
{
  struct termios tty;

  tcgetattr(STDIN_FILENO,&tty);

  if (enable)
  {
    tty.c_lflag |= ECHO;
  } // if
  else
  {
    tty.c_lflag &= ~ECHO;
  } // else

  tcsetattr(STDIN_FILENO,TCSANOW,&tty);

  return;

} // setEcho

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
  // Default to 8000 S/s.
  *parameters.sampleRatePtr = 8000;

  // Default for a 1000 millisecond signal.
  *parameters.durationPtr = 1000;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Set up for loop entry.
  done = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Retrieve the command line arguments.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  while (!done)
  {
    // Retrieve the next option.
    opt = getopt(argc,argv,"r:d:h");

    switch (opt)
    {
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
        fprintf(stderr,"./dtmf -r sampleRate -d duration, ms\n");

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
  int ch;
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
  parameters.sampleRatePtr = &sampleRate;
  parameters.durationPtr = &duration;

  // Retrieve the system parameters.
  exitProgram = getUserArguments(argc,argv,parameters);

  if (exitProgram)
  {
    // Bail out.
    return (0);
  } // if

  // Disable echo on stdin.
  setEcho(0);

  // We derive this converting milliseconds to seconds.
  numberOfSamples = (int)((sampleRate * duration / 1000));

  // Instantiate an NCO 1.
  myNco1Ptr = new Nco(sampleRate,440);

  // Instantiate an NCO 2.
  myNco2Ptr = new Nco(sampleRate,440);

  init((int)sampleRate);

  while (!exitProgram)
  {
    // Grab menu selection.
    fgets(textBuffer,5,stdin);

    switch (textBuffer[0])
    {
      case '1':
      {
        myNco1Ptr->setFrequency(700);
        myNco2Ptr->setFrequency(900);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case '2':
      {
        myNco1Ptr->setFrequency(700);
        myNco2Ptr->setFrequency(1100);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case '3':
      {
        myNco1Ptr->setFrequency(900);
        myNco2Ptr->setFrequency(1100);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case '4':
      {
        myNco1Ptr->setFrequency(700);
        myNco2Ptr->setFrequency(1300);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case '5':
      {
        myNco1Ptr->setFrequency(900);
        myNco2Ptr->setFrequency(1300);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case '6':
      {
        myNco1Ptr->setFrequency(1100);
        myNco2Ptr->setFrequency(1300);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case '7':
      {
        myNco1Ptr->setFrequency(700);
        myNco2Ptr->setFrequency(1500);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case '8':
      {
        myNco1Ptr->setFrequency(900);
        myNco2Ptr->setFrequency(1500);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case '9':
      {
        myNco1Ptr->setFrequency(1100);
        myNco2Ptr->setFrequency(1500);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case '0':
      {
        myNco1Ptr->setFrequency(1300);
        myNco2Ptr->setFrequency(1500);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case 'k':
      {
        myNco1Ptr->setFrequency(1100);
        myNco2Ptr->setFrequency(1700);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case 's':
      {
        myNco1Ptr->setFrequency(1500);
        myNco2Ptr->setFrequency(1700);
        amplitude1 = 0.35;
        amplitude2 = 0.35;
        break;
      } // case

      case 'h':
      {
        myNco1Ptr->setFrequency(2600);
        myNco2Ptr->setFrequency(0);
        amplitude1 = 0.80;
        amplitude2 = 0;
        break;
      } // case
    } // switch

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

      // Buffer it up!
      buf[i] = cosineValue;

    } // for

    // Write the samples to the sound caed.
    writePcmSamples(buf,numberOfSamples);

  } // while

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

void init(int sampleRate)
{
  int arg;
  int status;

  // Open sound device.
  fd = open("/dev/dsp",O_WRONLY);
  if (fd < 0)
  {
    perror("Open failed.");
  } // if

  // Set samping parameters.
  arg = SIZE;
  status = ioctl(fd,SOUND_PCM_WRITE_BITS,&arg);
  if (status == -1)
  {
    perror("ioctl(SOUND_PCM_WRITE_BITS) failed.");
  } // if
  if (arg != SIZE)
  {
    perror("Unable to set sample size.");
  } // if

  // Set number of channels.
  arg = CHANNELS;
  status = ioctl(fd,SOUND_PCM_WRITE_CHANNELS,&arg);
  if (status == -1)
  {
    perror("ioctl(SOUND_PCM_WRITE_CHANNELS) failed.");
  } // if
  if (arg != CHANNELS)
  {
    perror("Unable to set number of channels.");
  } // if

  // Set sample rate.
  arg = sampleRate;
  status = ioctl(fd,SOUND_PCM_WRITE_RATE,&arg);
  if (status == -1)
  {
    perror("ioctl(SOUND_PCM_WRITE_RATE) failed.");
  } // if
  if (arg != sampleRate)
  {
    perror("Unable to set sample rate.");
  } // if

  // Set data format.
  arg = AFMT_S16_LE;
  status = ioctl(fd,SOUND_PCM_SETFMT,&arg);
  if (status == -1)
  {
    perror("ioctl(SOUND_PCM_SETFMT) failed.");
  } // if

  return;

} // init


void writePcmSamples(int16_t *buffer,int numberOfSamples)
{
  int status;

  status = write(fd,buffer,numberOfSamples*2);

  // Wait for playback to complete.
  status = ioctl(fd,SOUND_PCM_SYNC);
  if (status == -1)
  {
    perror("ioctl(SOUND_PCM_SYNC) failed.");
  } // if

  return;

} // writePcmSamples

