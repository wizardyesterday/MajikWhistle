#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "MajikWhistle.h"

int main(int argc,char **argv)
{
  uint32_t i;
  MajikWhistle *myPtr;

  myPtr = new MajikWhistle(8000,NULL);

  // Set tone spacing to 0 milliseconds.
  myPtr->setToneSpacing(0);

  myPtr->dial("h");
  myPtr->dial("k");
  myPtr->dial("s");
  myPtr->dial("0");
  myPtr->dial("k12035551212s");

  usleep(1000000);

  if (myPtr != NULL)
  {
    delete myPtr;
  } // if

  return (0);

} // main
