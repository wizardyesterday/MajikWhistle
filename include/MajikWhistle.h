//**************************************************************************
// file name: MajikWhistle
//**************************************************************************
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// This class implements a device called a bluebox. This software has
// been provided for educational purposes and should not be used
// illegally
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#ifndef __MAJIKWHISTLE__
#define __MAJIKWHISTLE__

#include <stdint.h>
#include <map>

#include "Nco.h"
#include "PcmWriter.h"

struct toneInfo_t
{
  float frequency1;
  float amplitude1;
  float frequency2;
  float amplitude2;
};

class MajikWhistle
{
  //***************************** operations **************************

  public:

  MajikWhistle(uint32_t sampleRate,char *configurationFileNamePtr);

  ~MajikWhistle(void);

  void dial(char *phoneNumber);

  void setToneDuration(uint32_t durationInMilliseconds);
  void setToneSpacing(uint32_t dspacingInMilliseconds);

  private:

  //************************ utility functions ***********************
  void sendDigit(char digit);
  void sendSilence(void);

  //***************************** attributes **************************
  // The sample rate is needed when performing frequency changes.
  uint32_t sampleRate;

  // The number of samples for the tone duration.
  uint32_t numberOfSamplesForTone;

  // The number of samples for inter-digit delay.
  uint32_t numberOfSamplesForToneSpacing;

  // The the tone duration.
  uint32_t toneDurationInMilliseconds;

  // The inter-digit delay.
  uint32_t toneSpacingInMilliseconds;

  // Storage for tone samples and zero samples.
  int16_t toneBuffer[1024000];

  uint32_t toneBufferIndex;

  // Storage for zero-valued samples. This gives us 16 seconds.

  // These are the two oscillators whose outputs will be summed.
  Nco *nco1Ptr;
  Nco *nco2Ptr;

  // This sends PCM samples to the sound card.
  PcmWriter *pcmWriterPtr;

  std::map <char, toneInfo_t> toneMap;
};

#endif // __MAJIKWHISTLE__
