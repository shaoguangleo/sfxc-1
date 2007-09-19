/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef MPI_TRANSFER_H_
#define MPI_TRANSFER_H_

#include "types.h"
#include "sfxc_mpi.h"
#include "Delay_table_akima.h"

#include <Control_parameters.h>

/** Transfer special classes for the sfxc using MPI
 **/
class MPI_Transfer
{
public:
  MPI_Transfer();
  
  void send(Delay_table_akima &table, int sn, int rank);
  void receive(MPI_Status &status, Delay_table_akima &table, int &sn);

  void send(Track_parameters &track_param, int rank);
  void receive(MPI_Status &status, Track_parameters &track_param);
};

#endif /*MPI_TRANSFER_H_*/
