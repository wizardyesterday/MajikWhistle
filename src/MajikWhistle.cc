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

  Calling Sequence: MajikWhstle(sampleRate,fileNamePtr)

  Inputs:

    sampleRate - The sample rate in S/s.

    fileNamePtr - A pointer to a C string array that specifies the
    configuration file name.

  Outputs:

    None.

*****************************************************************************/
MajikWhistle::MajikWhistle(
  uint32_t sampleRate,
  char *configurationFileNamePtr)
{
  bool success;
  int i;

  // Default values.
  toneDurationInMilliseconds = 20;
  toneSpacingInMilliseconds = 100;

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

  Name: setToneDuration

  Purpose: The purpose of this function is to set the duration of a
  dial tone in milliseconds.
  
  Calling Sequence: setToneDuration(durationInMilliseconds)

  Inputs:

    durationInMilliseconds - The tone duration in milliseconds.

  Outputs:

    None.

*****************************************************************************/
void MajikWhistle::setToneDuration(uint32_t durationInMilliseconds)
{

  if (durationInMilliseconds > 1000)
  {
    // Limit it.
    durationInMilliseconds = 1000;
  } // if
  else
  {
    if (durationInMilliseconds <  20)
    {
      // Set it to a sane value.
      durationInMilliseconds = 20;
    } // if
  } // else

  toneDurationInMilliseconds = durationInMilliseconds;

  // Compute the number of samples for each tone.
  numberOfSamplesForTone = sampleRate * toneDurationInMilliseconds / 1000;

  return;

} // setToneDuration

/*****************************************************************************

  Name: setToneSpacing

  Purpose: The purpose of this function is to set the duration of a
  dial tone in milliseconds.
  
  Calling Sequence: setToneSpacing(spacingInMilliseconds)

  Inputs:

    spacingInMilliseconds - The tone spacing in milliseconds. If the
    value is zero, all tones will be sent with no spacing between them.

  Outputs:

    None.

*****************************************************************************/
void MajikWhistle::setToneSpacing(uint32_t spacingInMilliseconds)
{

  if (spacingInMilliseconds > 1000)
  {
    // Limit it.
    spacingInMilliseconds = 1000;
  } // if

  toneSpacingInMilliseconds = spacingInMilliseconds;

  // Compute the number of samples for the zero inter-digit sequence.
  numberOfSamplesForToneSpacing  =
    sampleRate * toneSpacingInMilliseconds / 1000;

  return;

} // setToneDuration

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
  uint32_t i;
  uint32_t length;

  // Do this for efficiency.
  length = strlen(phoneNumber);

  // We don't like zero-length strings.
  if (length > 0)
  {
    for (i = 0; i < length; i++)
    {
      sendDigit(phoneNumber[i]);
    } // for
  } // if

  return;

} // dial

/*****************************************************************************

  Name: sendDigit

  Purpose: The purpose of this function is to send the appropriate dial
  done the sound card.
  
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
  std::map <char, toneInfo_t>::iterator p;

  // Retrieve the tone parameters.
  p = toneMap.find(digit);

  // Did we find a match?
  if (p != toneMap.end())
  {
    // Retrieve th NCO parameters.
    frequency1 = p->second.frequency1;
    amplitude1 = p->second.amplitude1;
    frequency2 = p->second.frequency2;
    amplitude2 = p->second.amplitude2;

    for (i = 0; i < numberOfSamplesForTone; i++)
    {
     // Set the NCO's to the appropriate frequencies.
     nco1Ptr->setFrequency(frequency1);
     nco2Ptr->setFrequency(frequency2);

      // Get the next sample pair.
      nco1Ptr->runFast(&iValue,&qValue);

      // Convert to integer and scale.
      cosine1Value = (int16_t)(iValue * amplitude1 * 32767);

      // Get the next sample pair.
      nco2Ptr->runFast(&iValue,&qValue);

      // Convert to integer and scale.
      cosine2Value = (int16_t)(iValue * amplitude2 * 32767);

      // Sum and store.
      toneBuffer[i] = cosine1Value + cosine2Value;

    } // for

    // Send the PCM samples to the sound card. 
    pcmWriterPtr->writePcmSamples(toneBuffer,numberOfSamplesForTone);

    // Send inter-digit silence. No spacing between tones is allowed.
    if (numberOfSamplesForToneSpacing > 0)
    {
      pcmWriterPtr->writePcmSamples(zeroBuffer,numberOfSamplesForToneSpacing);
    } // if
  } // if

  return;

} // sendDigit

