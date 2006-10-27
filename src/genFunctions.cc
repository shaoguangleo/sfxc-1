/*
General purpose functions

Author     : RHJ Oerlemans
StartDate  : 04-08-2006
Last change: 04-08-2006

*/

#include "gen_defines.h"

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

//the class definitions and function definitions
#include "runPrms.h"
//global variables
extern RunP  RunPrms;

//*****************************************************************************
//continue or stop
//*****************************************************************************
void askContinue(void)
{
  char repl[2]; // user reply character
  
  printf("\nEnter c to continue, any other character to stop: ");
  scanf("%s",repl);
  if (strcmp(repl,"c")!=0) {
    printf("Application stopped by user!\n");
    exit(0);
  }
  printf("\n");
}




//*****************************************************************************
//
//*****************************************************************************
int getLongVal(char *key, char *val, char *skey, int& sval)
{
  char *endp;
  
  if (!strcmp(key,skey)) {
    sval = strtol(val, &endp, 10);
    if(RunPrms.get_messagelvl() > 1 ) {
      cout << "getLongVal: " << skey <<" "<< sval << endl;
    }
    if (endp == val) {
      fprintf(stderr,
        "**** Unable to convert string for key %s into long\n",key);
      return -1;
    }
  }
  return 0;
}



//*****************************************************************************
//
//*****************************************************************************
int getINT64Val(char *key, char *val, char *skey, INT64& sval)
{
  char *endp;
  
  if (!strcmp(key,skey)) {
    sval = strtol(val, &endp, 10);
    if(RunPrms.get_messagelvl() > 1 ) {
      cout << "getINT64Val: " << skey <<" "<< sval << endl;
    }
    if (endp == val) {
      fprintf(stderr,
        "**** Unable to convert string for key %s into long\n",key);
      return -1;
    }
  }
  return 0;
}


//*****************************************************************************
//
//*****************************************************************************
int getFloatVal(char *key, char *val, char *skey, float& sval)
{
  char *endp;
  
  if (!strcmp(key,skey)) {
    sval = strtod(val, &endp);
    if (endp == val) {
      fprintf(stderr,
        "**** Unable to convert string for key %s into float\n",key);
      return -1;
    }
  }
  return 0;
}


//*****************************************************************************
//
//*****************************************************************************
int str2int(char *val, int& sval)
{
  char *endp;
  
  sval = strtol(val, &endp, 10);
  if (endp == val) {
    fprintf(stderr,
      "**** Unable to convert string %s into long\n",val);
    return -1;
  }
  return 0;  
}



