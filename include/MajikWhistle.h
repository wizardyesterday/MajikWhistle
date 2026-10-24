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
#include "Nco.h"

class Nco
{
  //***************************** operations **************************

  public:

  MajikWhistle(float sampleRate,char *configurationFileNamePtr);
  ~MajikWhistle(void);

  void dial(std::string phoneNumber);
  void hangup(void);

  //***************************** attributes **************************
  private:

  // The sample rate is needed when performing frequency changes.
  float sampleRate;

  // These are the two oscillators whose outputs will be summed.
  Nco *nco1Ptr;
  Nco *nco2Ptr;
};

#endif // __MAJIKWHISTLE__
