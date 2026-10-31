/************************************************************************/
/* file name: diagUi.cc                                                 */
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <execinfo.h>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#include <sys/select.h>
#include <sys/time.h>

#include "MajikWhistle.h"
#include "diagUi.h"
#include "console.h"

using namespace std;

extern MajikWhistle *diagUi_whistlePtr;

/************************************************************************/
/* general defines                                                      */
/************************************************************************/

/************************************************************************/
/* structure definitions                                                */
/************************************************************************/
/* structure of entry in the user interface command table */
typedef struct _commandEntry
{
  const char *firstTokenPtr;            /* pointer to the command */
  const char *secondTokenPtr;           /* pointer to the subcommand */
  void (*handlerPtr)(char *bufferPtr); /* pointer to handler */
} commandEntry;

/************************************************************************/
/* global variables                                                     */
/************************************************************************/
static char commandBuffer[200];
static char lastCommandBuffer[200];
bool diagUi_timeToExit;
static pthread_t cliServerThread;

static console *consolePtr;
static bool consoleConnected;
static int connectFileDescriptor;
static int diagUiNetworkPort;

/**************************************************************************/
/* function prototypes                                                    */
/**************************************************************************/
/* command interpreter functions */
static void decodeCommand(char *commandBufferPtr);
static int newstrcmp(const char *string1Ptr,const char *string2Ptr);

static void getCommandTokens(char *bufferPtr,
                             char *firstTokenPtr,
                             char *secondTokenPtr,
                             char **parameterPtr);

/* command interpreter handlers */
static void cmdSetToneDuration(char *bufferPtr);
static void cmdSetToneSpacing(char *bufferPtr);
static void cmdDialCall(char *bufferPtr);
static void cmdSendHangup(char *bufferPtr);
static void cmdExitSystem(char *bufferPtr);
static void cmdHelp(void);
static void cmdDisplayInformation(char *bufferPtr);

/* network functions */
static void startCliServer(void *arg);
static void waitForCliConnection(int socketDescriptor);
static void serviceCliConnection(void);

// user interface thread entry point
static void userInterfaceProcedure(void *arg);

/************************************************************************/
/* configurable parameters                                              */
/************************************************************************/

/*************************************************************************/
/* The following table is the command table. The structure of each entry */
/* is a tuple as follows: {command, subcommand, handler}.  The last      */
/* entry in the table has zeros so that the decoder knows when the end   */
/* of the table has been reached.                                        */
/*************************************************************************/
static const commandEntry commandTable[] =
{ // command descriptors                             syntax
  {"set","toneduration",cmdSetToneDuration},   // set toneduration duration
  {"set","tonespacing",cmdSetToneSpacing},     // set tonespacing tonespacing
  {"dial","call",cmdDialCall},                 // dial call number
  {"send","hangup",cmdSendHangup},             // send hangup
  {"display","information",cmdDisplayInformation}, // display information
  {"exit","system",cmdExitSystem},             // exit system
  {"\0","\0",0}                                // last entry in command table
};

/*****************************************************************************

  Name: diagUi_start

  Purpose: The purpose of this function is to initialize the user interface
  subsystem and start it up.

  Calling Sequence: diagUi_start(networkPort)

  Inputs:

    networkPort - The TCP port for which a user is to connect.

  Outputs:

    None.

*****************************************************************************/
void diagUi_start(int networkPort)
{

  // this stops nprintf() from sending data on a nonexistant connection  
  consolePtr = 0;

  // save for later use
  diagUiNetworkPort = networkPort;

  // allow things to run
  diagUi_timeToExit = false;

  // create the user interface thread
  pthread_create(&cliServerThread,0,
                 (void *(*)(void *))startCliServer,0);

  return;

} // diagUi_start

/*****************************************************************************

  Name: diagUi_stop

  Purpose: The purpose of this function is to stop the user interface
  subsystem.

  Calling Sequence: diagUi_stop()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
void diagUi_stop()
{

  // notify the user interface thread to terminate
  diagUi_timeToExit = true;

  // we're done .. wait for the user interface thread to terminate
  pthread_join(cliServerThread,0);

  return;

} // diagUi_stop

/*****************************************************************************

  Name: decodeCommand

  Purpose: The purpose of this function is to decode a user command and
  execute the appropriate function. The structure of a command is as
  follows:

    command subcommand parameter1 parameter2 ... parameterN

  An example of a command would be: "set dac 3 225".
  The command is "set", the subcommand is "dac", and the parameters are
  3 and 225 corresponding to the channel number and DAC value, respectively.

  Calling Sequence: decodeCommand(commandBufferPtr)

  Inputs:

    commandBufferPtr - A pointer to a command buffer.

  Outputs:

    None.

*****************************************************************************/
static void decodeCommand(char *commandBufferPtr)
{
  int status, i;
  char firstToken[80], secondToken[80], *parameterPtr;
  unsigned char found, done;

  /* get the first two tokens associated with the command */
  getCommandTokens(commandBufferPtr,firstToken,secondToken,&parameterPtr);

  status = newstrcmp("help",firstToken);
  
  if (status == 0)
  { /* help command */
    cmdHelp();
    return;
  } /* if */

  i = -1; /* reference one element BEFORE the command table */
  done = 0; /* initialize loop flag */
  found = 0; /* set to a 'not found' condition */

  while (done == 0)
  { /* scan the command table for a command match */
    i++; /* reference next entry in the command table */
    if (commandTable[i].firstTokenPtr[0] == '\0')
    { /* we're at the end of the table */
      done = 1; /* bail out of loop */
    } /* if */
    else
    { /* we have a command to compare */
      status = newstrcmp(commandTable[i].firstTokenPtr,firstToken);
      if (status == 0)
      { /* we have a match on the first token */
        status = newstrcmp(commandTable[i].secondTokenPtr,secondToken);
        if (status == 0)
        { /* we have a match on both tokens */
          found = 1; /* indicate our match! */
          done = 1; /* bail out of loop */
        } /* if */
      } /* if */
    } /* else */
  } /* while */

  if (found == 1)
  { /* we have a valid command */
    commandTable[i].handlerPtr(parameterPtr);
  } /* if */
  else
  { /* we have an invalid command */
    nprintf(stderr,
            "\n\nINVALID COMMAND - - type help for a list of commands\n\n\r");
  } /* else */

  return;

} /* decodeCommand */

/*****************************************************************************

  Name: getCommandTokens

  Purpose: The purpose of this function is to get the first tokens from
  the command buffer and store them as NULL terminated strings.  In
  addition, a pointer to the buffer parameters is returned.

    command subcommand parameter1 parameter2 ... parameterN

  An example of a command would be: "set dac 3 225".
  The first token is "set", the second token is "dac", and the parameters
  are 3 and 225 corresponding to the channel number and DAC value,
  respectively. So, firstTokenPtr would reference "set", secondTokenPtr
  would reference "dac", and parameterPtr would reference the space
  character preceeding the number 3.

  Calling Sequence: getCommandTokens(bufferPtr,
                                     firstTokenPtr,
                                     secondTokenPtr,
                                     &parameterPtr)

  Inputs:

    bufferPtr - A pointer to the command bufer.

    firstTokenPtr - A pointer to memory which accepts the first token.

    secondTokenPtr - A pointer to memory which accepts the second token.

  Outputs:

    firstTokenPtr - A pointer to memory which accepts the first token.

    secondTokenPtr - A pointer to memory which accepts the second token.

    parameterPtr - A pointer to the parameters in the command buffer.

*****************************************************************************/
static void getCommandTokens(char *bufferPtr,
                             char *firstTokenPtr,
                             char *secondTokenPtr,
                             char **parameterPtr)
{

  *firstTokenPtr = '\0'; /* protect ourselves */
  *secondTokenPtr = '\0'; /* protect ourselves */

  while ((*bufferPtr == ' ') && (*bufferPtr != '\0'))
  {
    bufferPtr++; /* move past any leading white space */
  } /* while */

  /* construct first token */
  while ((*bufferPtr != ' ') && (*bufferPtr != '\0'))
  {
    *firstTokenPtr = *bufferPtr++; /* get first token */
    *firstTokenPtr |= 0x60; /* convert to lower case */
    firstTokenPtr++; /* reference next storage location */
  } /* while */

  while ((*bufferPtr == ' ') && (*bufferPtr != '\0'))
  {
    bufferPtr++; /* move past white space */
  } /* while */

  while ((*bufferPtr != ' ') && (*bufferPtr != '\0'))
  {
    *secondTokenPtr = *bufferPtr++; /* get second token */
    *secondTokenPtr |= 0x60; /* convert to lower case */
    secondTokenPtr++; /* reference next storage location */
  } /* while */

  *firstTokenPtr = '\0'; /* terminate our string for the first token */
  *secondTokenPtr = '\0'; /* terminate our string for the second token */
  *parameterPtr = bufferPtr; /* reference parameters in buffer */

  return;

} /* getCommandTokens */

/**************************************************************************

  Name: newstrcmp

  Purpose: The purpose of this function is to compare two strings.

  Calling Sequence: status = newstrcmp(s1,s2)

  Inputs:

    s1 - A pointer to the first string to be compared.
    s2 - A pointer to the second string to be compared.

  Outputs:

    status - The result of the comparison.

**************************************************************************/
static int newstrcmp(const char *s1,const char *s2)
{
  while( (*s1 == *s2) && (*s1) ) ++s1, ++s2;

  return ((int)(unsigned char)*s1) - ((int)(unsigned char)*s2);
}

/*****************************************************************************

  Name: cmdSetToneDuration

  Purpose: The purpose of this function is to set the duration, in milliseconds,
  of a dial tone.

  The syntax for the corresponding command is the following:

    "set toneduration durationInMilliseconds"

  Calling Sequence: cmdSetToneDuration(bufferPtr)

  Inputs:

    bufferPtr - A pointer to the command parameters.

  Outputs:

    None.

*****************************************************************************/
static void cmdSetToneDuration(char *bufferPtr)
{
  uint32_t duration;

  // Retrieve value
  sscanf(bufferPtr,"%u",&duration);

  // Set the demodulator mode.
  diagUi_whistlePtr->setToneDuration(duration);

  nprintf(stderr,"Tone duration set to %u milliseconds.\n",duration);

  return;

} // cmdSeToneDuration

/*****************************************************************************

  Name: cmdSetToneSpacing

  Purpose: The purpose of this function is to set the spacing between dial
  tones.

  The syntax for the corresponding command is the following:

    "set tonespacing spacingInMilliseconds"

  Calling Sequence: cmdSetToneSpacing(bufferPtr)

  Inputs:

    bufferPtr - A pointer to the command parameters.

  Outputs:

    None.

*****************************************************************************/
static void cmdSetToneSpacing(char *bufferPtr)
{
  uint32_t spacing;

  // Retrieve value
  sscanf(bufferPtr,"%u",&spacing);

  // Set the tone spacing.
  diagUi_whistlePtr->setToneSpacing(spacing);;

  nprintf(stderr,"Tone spacing set to %u milliseconds.\n",spacing);

  return;

} // cmdSetToneSpacing

/*****************************************************************************

  Name: cmdDialCall

  Purpose: The purpose of this function is to send dial tones to make a call.

  The syntax for the corresponding command is the following:

    "dial number numberstring"

  Calling Sequence: cmdDialNumber(bufferPtr)

  Inputs:

    bufferPtr - A pointer to the command parameters.

  Outputs:

    None.

*****************************************************************************/
static void cmdDialCall(char *bufferPtr)
{
  int i;
  int length;

  length = strlen(bufferPtr);

  while (bufferPtr[i] == ' ')
  {
    // Advanxe past leading spaces.
    bufferPtr++;
  } // for

  // Send the number to dial.
  diagUi_whistlePtr->dial(bufferPtr);

  nprintf(stderr,"Phone number %s dialed.\n",bufferPtr);

  return;

} // cmdDialCall


/*****************************************************************************

  Name: cmdSendHangup

  Purpose: The purpose of this function is to send dial tones to make a call.

  The syntax for the corresponding command is the following:

    "send hangup"

  Calling Sequence: cmdSendHangup(bufferPtr)

  Inputs:

    bufferPtr - A pointer to the command parameters.

  Outputs:

    None.

*****************************************************************************/
static void cmdSendHangup(char *bufferPtr)
{

  // Send the hangup.
  diagUi_whistlePtr->dial("h");

  nprintf(stderr,"Hangup sent\n");

  return;

} // cmdSendHangup

/*****************************************************************************

  Name: cmdSendHangup

  Purpose: The purpose of this function is to send dial tones to make a call.

  The syntax for the corresponding command is the following:

    "send hangup"

  Calling Sequence: cmdSendHangup(bufferPtr)

  Inputs:

    bufferPtr - A pointer to the command parameters.

  Outputs:

    None.

*****************************************************************************/
static void cmdDisplayInformation(char *bufferPtr)
{
  char displayBuffer[128000];
  char *displayBufferPtr;

  displayBufferPtr = displayBuffer;

  // Send the hangup.
  diagUi_whistlePtr->displayInternalInformation(&displayBufferPtr);

  nprintf(stderr,"%s",displayBuffer);

  return;

} // cmdDisplayIndormation

/*****************************************************************************

  Name: cmdExitSystem

  Purpose: The purpose of this function is to exit the system.

  The syntax for the command is as follows:

    "exit system"

  Calling Sequence: cmdExitSystem()

  Inputs:

    bufferPtr - A pointer to the command parameters.

  Outputs:

    None.

*****************************************************************************/
static void cmdExitSystem(char *bufferPtr)
{

  // indicate that it's time to exit the system
  diagUi_timeToExit = true;

  return;

} /* cmdExitSystem */

/*****************************************************************************

  Name: cmdHelp

  Purpose: The purpose of this function is to display the help information
  to the user.

  The syntax for the command is as follows:

    "help"

  Calling Sequence: cmdHelp()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
static void cmdHelp(void)
{

  nprintf(stderr,"set toneduration <duration (milliseconds)>\n");
  nprintf(stderr,"set tonespacing <spacing (milliseconds)>\n");
  nprintf(stderr,"dial call <<k>number<s>>\n");
  nprintf(stderr,"send hangup\n");
  nprintf(stderr,"display information\n");
  nprintf(stderr,"exit system\n");
  nprintf(stderr,"help\n");
  nprintf(stderr,"Type <^B><enter> key sequence to repeat last command\n");

  return;

} /* cmdHelp */

/*****************************************************************************

  Name: nprintf

  Purpose: The purpose of this function is to start the command line
  interpreter server.

  Calling Sequence: nprint(s,formatPtr,arg1, arg2..)

  Inputs:

    s - A file pointer normally used by nprintf().  This is a dummy
    argument.

    formatPtr - A pointer to a printf()-style format string.

    arg1,arg2,... - The arguments that are to be printed.

  Outputs:

    None.

*****************************************************************************/
void nprintf(FILE *s,const char *formatPtr, ...)
{
  char buffer[2048];
  va_list args;


  if (consolePtr != 0)
  {
    // set up for argument retrieval  
    va_start(args,formatPtr);

    // store the formated data to a string
    vsprintf(buffer,formatPtr,args);

    // we're done with the args
    va_end(args);

    // display the information to the user
    consolePtr->putLine(buffer);
  } // if

} // nprintf

/*****************************************************************************

  Name: startCliServer

  Purpose: The purpose of this function is to start the command line
  interpreter server.  This is the entry point to the server thread.
  This function listens on a TCP socket and waits for a connection
  from a user.  When a connection is established, the user interface
  thread is launched, and this function will block until the user
  interface thread is terminated.  Once the user interface thread is
  terminated, this function will close its half of the client connection
  and will wait for a new connection.

  Calling Sequence: startCliServer(arg)

  Inputs:

    arg - A pointer to an argument.

  Outputs:

    None.

*****************************************************************************/
static void startCliServer(void *arg)
{
  int status, option;
  socklen_t clientAddressLength;
  struct sockaddr_in addr;
  int fileDescriptor;

  // This is used for setting socket options.
  option = 1;

  // Create socket.
  fileDescriptor = socket(PF_INET,SOCK_STREAM,0);

  //*******************************************************************
  // If the file descriptor is valid, a socket has been created.  Now
  // we set up the parameters that are needed for the bind() call.
  //*******************************************************************
  if (fileDescriptor != -1)
  {
    // Set up to reuse the port number.
    status = setsockopt(fileDescriptor,SOL_SOCKET,SO_REUSEADDR,
        (char *)&option,sizeof(option));

    if (status != -1)
    {
      // Set up the address structure.
      bzero((char *)&addr,sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = inet_addr("0.0.0.0");
      addr.sin_port = htons(diagUiNetworkPort);

      // Associate the address with the socket. 
      status = bind(fileDescriptor,(struct sockaddr *)&addr,sizeof(addr));
    } // if

    if (status != -1)
    {
      // Accept only one connection.
      status = listen(fileDescriptor,1);
    } // if

   /* This is needed needed by the accept() call. */
    clientAddressLength = sizeof(addr);

    //**********************************************************************
    // This is the main server loop.  The server will block on the
    // accept() call until a connection is made by a client.  Once a
    // connection is established, the connection will be serviced, and no
    // other processing will occur.  Note that this loop is traversed only
    // once per connect/disconnect sequence, since this is an interative
    // server that services only one client connection at a time.
    //**********************************************************************
    while (!diagUi_timeToExit)
    {
      // Indicate that there is no network connection.
      consoleConnected = false;

      // Wait until a client is attempting to connect.
      waitForCliConnection(fileDescriptor);

      if (!diagUi_timeToExit)
      {
        //************************************************************
        // This block of code will accept the incoming connection.
        // If the system call is interrupted for some reason, another
        // attempt will be made to accept connections.
        //************************************************************
        do
        {
          // Accept the incoming connection.
          connectFileDescriptor = 
             accept(fileDescriptor,(sockaddr *)&addr,&clientAddressLength);

          if (connectFileDescriptor == -1)
          {
            if (errno == EINTR)
            {
              continue;
            } /* if */
          } // if
        } // do
        while (connectFileDescriptor == -1);

        // Do the appropriate processing for the new connection.
        serviceCliConnection();

        // We're done with this connection.
        close (connectFileDescriptor);

      } // if

    } // while

  } // if
  else
  {
    // clean up the mess
    close(fileDescriptor);
  } // else

  if (fileDescriptor != -1)
  {
    close(fileDescriptor);
  } // if

  pthread_exit(0);

} // startCliServer

/*****************************************************************************

  Name: waitForCliConnection

  Purpose: The purpose of this function is to wait for a client
  connection.

  Calling Sequence: waitForCliConnection(socketDescriptor)

  Inputs:

    socketDescriptor - A descriptor for the listener socket..

  Outputs:

    None.

*****************************************************************************/
static void waitForCliConnection(int socketDescriptor)
{
  bool done;
  int status;
  fd_set connectSet;
  struct timeval timeout;

  // Set up for loop entry.
  done = false;

  while ((!diagUi_timeToExit) && (!done))
  {
    // Ensure that we're clean.
    FD_ZERO(&connectSet);

    // Indicate the descriptor of interest.
    FD_SET(socketDescriptor,&connectSet);

    // Set timeout to 1 second.
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    // See which descriptors are available for reading.
    status = select(socketDescriptor+1,&connectSet,0,0,&timeout);

    if (status != -1)
    {
      if (FD_ISSET(socketDescriptor,&connectSet))
      {
        // Bail out.
        done = true;
      } // if
    } // if
  } // while

  return;

} // waitForCliConnection

/*****************************************************************************

  Name: serviceCliConnection

  Purpose: The purpose of this function is to service a command line
  interface user session.  The user interface thread is launched, and the
  function will wait either on an indication of time to exit the system or
  a network connection terminated by the link partner.

  Calling Sequence: cliServer()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
static void serviceCliConnection(void)
{
  struct timeval timeout;
  pthread_t userInterfaceThread;

  // Indicate that the console is connected.
  consoleConnected = true;

  // Create the user interface thread.
  pthread_create(&userInterfaceThread,0,
                 (void *(*)(void *))userInterfaceProcedure,0);

  // Wait until either it is time to exit or the connection is dropped by
  // the link partner.
  while ((consoleConnected) &&
         (!diagUi_timeToExit))
  {
    // Set up to block for 100ms.
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    // Wait for the designated time interval.
    select(0,0,0,0,&timeout);
  } // while

  // Ok we're done .. wait for the user interface thread to terminate.
  pthread_join(userInterfaceThread,0);

  return;

} // serviceCliConnection

/*****************************************************************************

  Name: userInterfaceProcedure

  Purpose: The purpose of this function is to provide user interface
  functionality of the system.  This function is the entry point to the
  user interface thread.

  Calling Sequence: userInterfaceProcedure(arg)

  Inputs:

    arg - A pointer to an argument.

  Outputs:

    None.

*****************************************************************************/
static void userInterfaceProcedure(void *arg)
{
  unsigned short length;
  unsigned short i;

  // create a console
  consolePtr = new console(connectFileDescriptor,
                                &consoleConnected);

  // display introduction message
  nprintf(stderr,"\nWelcome to System Diagnostics\n");

  while (!diagUi_timeToExit && consoleConnected)
  {
    length = consolePtr->getLine(commandBuffer,80);

    if (length != 0)
    { // we have a command
      if (commandBuffer[0] != 2)
      { // new command

        for (i = 0; i <= length; i++)
        {
          lastCommandBuffer[i] = commandBuffer[i];
        }// for 
        /* execute the new command */
        decodeCommand(commandBuffer);
      } /* if */
      else
      { // repeat last command
        decodeCommand(lastCommandBuffer);
      } // else

    } // if
  } // while */

  // we don't need this anymore
  delete consolePtr;
  consolePtr = 0;

  pthread_exit(0);

} // userInterfaceProcedure

