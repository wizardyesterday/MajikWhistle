//************************************************************************
// file name: MajikWhstle.cc
//************************************************************************
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "Nco.h"
#include "PcmWriter.h"
#include "MajikWhistle.h"

using namespace std;


struct toneInfoMapping_t
{
  char key;
  toneInfo_t ncoInfo;
};

  toneInfoMapping_t a[] =
{
  {'1', {700, 0.35, 900, 0.35}},
  {'2', {700, 0.35, 1100, 0.35}},
  {'3', {900, 0.35, 1100, 0.35}},
  {'4', {700, 0.35, 1300, 0.35}},
  {'5', {900, 0.35, 1300, 0.35}},
  {'6', {1100, 0.35, 1300, 0.35}},
  {'7', {700, 0.35, 1500, 0.35}},
  {'8', {900, 0.35, 1500, 0.35}},
  {'9', {1100, 0.35, 1500, 0.35}},
  {'0', {1300, 0.35, 1500, 0.35}},
  {'k', {1100, 0.35, 1700, 0.35}},
  {'k', {1500, 0.35, 1700, 0.35}},
  {'h', {2600, 0.70, 0, 0}},
};

/*****************************************************************************

  Name: MajikWhstle

  Purpose: The purpose of this function is to serve as the constructor for
  an instance of a MajikWhstle.

  Calling Sequence: MajikWhstle(sampleRate)

  Inputs:

    sampleRate - The sample rate in S/s.

  Outputs:

    None.

*****************************************************************************/
MajikWhistle::MajikWhistle(int sampleRate,char *conifigurationFilePtr)
{
  bool success;

  // Instantiate the NCO's.
  nco1Ptr = new Nco((float)sampleRate,2600);
  nco2Ptr = new Nco((float)sampleRate,2600);

  // Instantiate a PCM writer.
  pcmWriterPtr = new PcmWriter(sampleRate,&success);

} // MajikWhstle

/*****************************************************************************

  Name: ~MajikWhstle

  Purpose: The purpose of this function is to serve as the destructor for
  an instance of a MajikWhstle.

  Calling Sequence: ~MajikWhstle()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
MajikWhistle::~MajikWhistle(void)
{

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Release resources.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  if (nco1Ptr != NULL)
  {
    delete  nco1Ptr;
  } // if

  if (nco2Ptr != NULL)
  {
    delete  nco2Ptr;
  } // if

  if (pcmWriterPtr != NULL)
  {
    delete pcmWriterPtr;
  } // if
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

} // MajikWhstle

/*****************************************************************************

  Name: dial

  Purpose: The purpose of this function is to dial a phone number using
  a touch tone sequence.
  
  Calling Sequence: dial(*phoneNumberPtr)

  Inputs:

    phoneNumberPtr - A pointer  to a C-terminated string of characters.

  Outputs:

    None.

*****************************************************************************/
void MajikWhistle::dial(char *phoneNumber)
{

  return;

} // dial

/*****************************************************************************

  Name: hangup

  Purpose: The purpose of this function is to dial send a hangup tone.
  
  Calling Sequence: dial(void)

  Inputs:

    None.

  Outputs:

    Nonw.

*****************************************************************************/
void MajikWhistle::hangup(void)
{

  return;

} // hangup

