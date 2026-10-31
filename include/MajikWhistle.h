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

#define SAMPLE_RATE (8000)
#define TONE_TIME (1)
#define SILENT_TIME (1)
#define SINGLE_ENTRY_SIZE ((TONE_TIME + SILENT_TIME) * SAMPLE_RATE)
#define MAXIMUM_PHONE_NUMBER_SIZE (20)
#define TONE_BUFFER_SIZE (SINGLE_ENTRY_SIZE * MAXIMUM_PHONE_NUMBER_SIZE)

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
  void displayInternalInformation(char **displayBufferPtrPtr);

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
  int16_t toneBuffer[TONE_BUFFER_SIZE];

  uint32_t toneBufferIndex;

  // These are the two oscillators whose outputs will be summed.
  Nco *nco1Ptr;
  Nco *nco2Ptr;

  // This sends PCM samples to the sound card.
  PcmWriter *pcmWriterPtr;

  std::map <char, toneInfo_t> toneMap;
};

#endif // __MAJIKWHISTLE__
