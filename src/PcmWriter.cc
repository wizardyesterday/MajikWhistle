//************************************************************************
// file name: PcmWriter.cc
//************************************************************************
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#include "PcmWriter.h"

using namespace std;

#define SIZE (16)
#define CHANNELS (1)

/*****************************************************************************

  Name: PcmWriter

  Purpose: The purpose of this function is to serve as the constructor for
  an instance of a PcmWriter.

  Calling Sequence: PcmWriter(sampleRate,frequency);

  Inputs:

    sampleRate - The sample rate in S/s.

    frequency - The frequency in Hz.

  Outputs:

    None.

*****************************************************************************/
PcmWriter::PcmWriter(int sampleRate,bool *successPtr)
{
  int arg;
  int status;

  // Default to success.
  *successPtr = true;

  // Open sound device.
  fd = open("/dev/dsp",O_WRONLY);

  if (fd < 0)
  { 
    perror("Open failed.");

    // indicate failure.
    *successPtr = false;
  } // if
  
  // Set samping parameters.
  arg = SIZE;
  status = ioctl(fd,SOUND_PCM_WRITE_BITS,&arg);

  if (status == -1)
  {
    perror("ioctl(SOUND_PCM_WRITE_BITS) failed.");

    // indicate failure.
    *successPtr = false;
  } // if
  if (arg != SIZE)
  {
    perror("Unable to set sample size.");

    // indicate failure.
    *successPtr = false;
  } // if

  // Set number of channels.
  arg = CHANNELS;
  status = ioctl(fd,SOUND_PCM_WRITE_CHANNELS,&arg);

  if (status == -1)
  {
    perror("ioctl(SOUND_PCM_WRITE_CHANNELS) failed.");

    // indicate failure.
    *successPtr = false;
  } // if
  if (arg != CHANNELS)
  {
    perror("Unable to set number of channels.");

    // indicate failure.
    *successPtr = false;
  } // if

  // Set sample rate.
  arg = sampleRate;
  status = ioctl(fd,SOUND_PCM_WRITE_RATE,&arg);

  if (status == -1)
  {
    perror("ioctl(SOUND_PCM_WRITE_RATE) failed.");

    // indicate failure.
    *successPtr = false;
  } // if
  if (arg != sampleRate)
  {
    perror("Unable to set sample rate.");

    // indicate failure.
    *successPtr = false;
  } // if

  // Set data format.
  arg = AFMT_S16_LE;
  status = ioctl(fd,SOUND_PCM_SETFMT,&arg);

  if (status == -1)
  {
    perror("ioctl(SOUND_PCM_SETFMT) failed.");

    // indicate failure.
    *successPtr = false;
  } // if

} // PcmWriter

/*****************************************************************************

  Name: ~PcmWriter

  Purpose: The purpose of this function is to serve as the destructor for
  an instance of a PcmWriter.

  Calling Sequence: ~PcmWriter()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
PcmWriter::~PcmWriter(void)
{

} // PcmWriter

/*****************************************************************************

  Name: writePcmSamples

  Purpose: The purpose of this function is to reset all runtime values to
  initial values.

  Calling Sequence: writePcmSamples(bufferPtr,numberOfSamples)

  Inputs:

    bufferPtr - A pointer to a buffer that contains PCM samples.

    numberOfSamples - The number of PCM samples contained in the buffer,
    referenced by bufferPtrr

  Outputs:

    success - A flag that ehen set to true, indicates success, and when
    set to false, indicates failure.

*****************************************************************************/
bool PcmWriter::writePcmSamples(int16_t *bufferPtr,uint32_t numberOfSamples)
{
  int status;
  bool success;

  // Defaulr to failure.
  success = false;

  // Write the PCM samples to the sound card.
  status = write(fd,bufferPtr,(numberOfSamples * 2));

  if (status != -1)
  {
    // Wait for playback to complete.
    status = ioctl(fd,SOUND_PCM_SYNC);
  } // if

  if (status != -1)
  {
    // Indicate success.
    success = true;
  } // if

  return (success);

} // writePcmSamples

