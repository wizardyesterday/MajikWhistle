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

  toneInfoMapping_t ncoParameters[] =
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

  Calling Sequence: MajikWhstle(sampleRate,toneDurationInMilliseconds,
                                toneSpacingInMilliseconds,
                                fileNamePtr)

  Inputs:

    sampleRate - The sample rate in S/s.

    toneDurationInMilliseconds - The duration of each tone.

    toneSpacingInMilliseconds - The time spacing between each tone.

    fileNamePtr - A pointer to a C string array that specifies the
    configuration file name.

  Outputs:

    None.

*****************************************************************************/
MajikWhistle::MajikWhistle(
  int sampleRate,
  int toneDurationInMilliseconds,
  int toneSpacingInMilliseconds,
  char *configurationFileNamePtr)
{
  bool success;
  int i;

  if (toneDurationInMilliseconds > 2000)
  {
    // Limit it to 2 seconds.
    toneDurationInMilliseconds = 2000;
  } // if

  if (toneSpacingInMilliseconds > 4000)
  {
    // Limit it to 4 seconds.
    toneSpacingInMilliseconds = 4000;
  } // if

  // Compute the number of samples for each tone.
  numberOfSamplesForTone = sampleRate * toneDurationInMilliseconds / 1000;

  // Compute the number of samples for the zero inter-digit sequence.
  numberOfSamplesForToneSpacing  =
    sampleRate * toneSpacingInMilliseconds / 1000;

  // Fill the digit spacing buffer.
  for (i = 0; i < numberOfSamplesForToneSpacing; i++)
  {
    zeroBuffer[i] = 0;
  } // for

  // Populate the tone map.
  for (i = 0; i < sizeof(ncoParameters); i++)
  {
    toneMap[ncoParameters[i].key] = ncoParameters[i].ncoInfo;
  } // for

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
  int i;
  int length;

  // Do this for efficiency.
  length = strlen(phoneNumber);

  for (i = 0; i < length; i++)
  {
    sendDigit(phoneNumber[i]);
  } // for

  return;

} // dial

/*****************************************************************************

  Name: sendDigit

  Purpose: The purpose of this function is to dial a phone number using
  a touch tone sequence.
  
  Calling Sequence: sendDigit(digit)

  Inputs:

    digit - A character that represents a digit.

  Outputs:

    None.

*****************************************************************************/
void MajikWhistle::sendDigit(char digit)
{
  int i;
  float frequency1;
  float frequency2;
  float amplitude1;
  float amplitude2;
  int16_t cosine1Value;
  int16_t cosine2Value;
  int16_t cosineValue;
  float iValue;
  float qValue;

  // Retrieve the tone parameters.

  for (i = 0; i < numberOfSamplesForTone; i++)
  {
   // Do the majik. 
  } // for

  // Send the PCM samples to the sound card. 
  pcmWriterPtr->writePcmSamples(toneBuffer,numberOfSamplesForTone);

  return;

} // dial

