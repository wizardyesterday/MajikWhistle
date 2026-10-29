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

  MajikWhistle(int sampleRate,
               int toneDurationInMilliseconds,
               int toneSpacingInMilliseconds,
               char *configurationFileNamePtr);

  ~MajikWhistle(void);

  void dial(char *phoneNumber);

  private:

  //************************ utility functions ***********************
  void sendDigit(char digit);

  //***************************** attributes **************************

  // The sample rate is needed when performing frequency changes.
  int sampleRate;

  // Thenumber of samples for the tone duration.
  int numberOfSamplesForTone;

  // The number of samples for inter-digit delay.
  int numberOfSamplesForToneSpacing;

  // Storage for tone samples. This gives us 16 seconds.
  int16_t toneBuffer[256000];

  // Storage for zero-valued samples. This gives us 16 seconds.
  int16_t zeroBuffer[256000];

  // These are the two oscillators whose outputs will be summed.
  Nco *nco1Ptr;
  Nco *nco2Ptr;

  // This sends PCM samples to the sound card.
  PcmWriter *pcmWriterPtr;

  std::map <char, toneInfo_t> toneMap;
};

#endif // __MAJIKWHISTLE__
