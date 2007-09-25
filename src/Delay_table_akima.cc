/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans  <Oerlemans@JIVE.nl>, 2007
 *            Nico Kruithof   <Kruithof@JIVE.nl>, 2007
 *            Huseyin Ozdemir <Ozdemir@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Class function definitions for station specific data
 */

//standard c includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//includes for system calls
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

//c++ includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
using namespace std;

//the class definitions and function definitions
#include "constPrms.h"
#include "runPrms.h"
#include "genPrms.h"
#include "staPrms.h"
#include "genFunctions.h"
#include "Delay_table_akima.h"
#include <utils.h>

//*****************************************************************************
//function definitions
//*****************************************************************************

//default constructor, set default values
Delay_table_akima::Delay_table_akima() 
  : end_scan(0), acc(NULL), splineakima(NULL)
{
}

//destructor
Delay_table_akima::~Delay_table_akima()
{
}

bool Delay_table_akima::operator==(const Delay_table_akima &other) const
{
  return true;
}

//set values for delay table columns
void Delay_table_akima::set_cmr(GenP GenPrms)
{
  cde = GenPrms.get_cde();
  mde = GenPrms.get_mde();  
  rde = GenPrms.get_rde();  
}


//read the delay table, do some checks and
//calculate coefficients for parabolic interpolation
int Delay_table_akima::open(const char *delayTableName)
{
  std::ifstream in(delayTableName);
  assert(in.is_open());
  int32_t header_size;
	
  // Read the header
  in.read(reinterpret_cast < char * > (&header_size), sizeof(int32_t));
  char header[header_size];
  in.read(reinterpret_cast < char * > (header), header_size*sizeof(char));
   
  // Read the data
  double line[5];
  while (in.read(reinterpret_cast < char * > (line), 5*sizeof(double))){
    // The time read from file is in seconds, whereas the software correlator
    // works with times in microseconds
    times.push_back(line[0]*1000000);
    delays.push_back(line[4]);
  }

  // Initialise
  end_scan = 0;
  initialise_spline_for_next_scan();

  return 0;
}

void Delay_table_akima::initialise_spline_for_next_scan() {
  assert(end_scan < times.size()-1);

  // make end_scan point to the start of the next scan
  if (end_scan != 0) end_scan += 2;

  // next_end_scan is the past-the-end iterator of the next scan
  size_t next_end_scan = end_scan;
  while ((next_end_scan < times.size()) && 
         (times[next_end_scan] != 0)) {
    next_end_scan ++;
  }
  assert(next_end_scan < times.size());

  if (splineakima != NULL) {
    gsl_spline_free(splineakima);
    gsl_interp_accel_free(acc);
  }

  // Initialise the Akima spline
  acc = gsl_interp_accel_alloc();
  int n_pts = next_end_scan - end_scan;
  // at least 4 sample points for a spline
  assert(n_pts >= 4);

  // End scan now points to the beginning of the next scan and 
  // the next scan has n_pts data points
  splineakima = gsl_spline_alloc(gsl_interp_akima, n_pts);

  gsl_spline_init(splineakima,
                  &times[end_scan], 
                  &delays[end_scan], 
                  n_pts);
  
  end_scan = next_end_scan-1;
}


//calculates the delay for the delayType at time in microseconds
//get the next line from the delay table file
double Delay_table_akima::delay(int64_t time, int delayType) {
  while (times[end_scan] < time) initialise_spline_for_next_scan();
  return gsl_spline_eval (splineakima, time, acc);
}
