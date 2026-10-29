//**************************************************************************
// file name: PcmWriter.h
//**************************************************************************
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// This class implements a writer that writes PCM samples to a sound
// card by accessing the appropriate kernel deiver.
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#ifndef __PCMWRITER__
#define __PCMWRITER__

#include <stdint.h>

class PcmWriter
{
  //***************************** operations **************************

  public:

  PcmWriter(int sampleRate,bool *successPtr);
  ~PcmWriter(void);

  bool writePcmSamples(int16_t *bufferPtr,uint32_t numberOfSamples);

  //***************************** attributes **************************
  private:

  // file descriptor for DSP device.
  int fd;
};

#endif // __PCMWRITER__
