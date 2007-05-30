/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef SINGLE_DATA_WRITER_CONTROLLER_H
#define SINGLE_DATA_WRITER_CONTROLLER_H

#include <Controller.h>

#include <Data_writer.h>
#include <Semaphore_buffer.h>
#include <Buffer2data_writer.h>

class Single_data_writer_controller : public Controller {
  typedef Single_data_writer_controller  Self;
public:
  typedef Buffer_element<char,131072>      value_type;
  typedef Buffer<value_type>               Buffer;
  
  Single_data_writer_controller(Node &node);
  ~Single_data_writer_controller();

  Process_event_status process_event(MPI_Status &status);
  
  Buffer *buffer();
  void set_buffer(Buffer *buffer);
  Data_writer *get_data_writer(int i);
private:
  void set_data_writer(Data_writer *writer);

  Buffer2data_writer<value_type>                 buffer2writer;
};

#endif /* SINGLE_DATA_WRITER_CONTROLLER_H */