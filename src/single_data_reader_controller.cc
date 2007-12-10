/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include "single_data_reader_controller.h"
#include "sfxc_mpi.h"
#include "data_reader_factory.h"
#include "data_reader_tcp.h"

Single_data_reader_controller::
Single_data_reader_controller(Node &node) 
  : Controller(node) {
}

Single_data_reader_controller::Process_event_status
Single_data_reader_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_ADD_DATA_READER_FILE2:
    {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      
      int size;
      MPI_Get_elements(&status, MPI_CHAR, &size);
      assert(size > 0);
      char msg[size];
      MPI_Recv(&msg, size, MPI_CHAR, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      int32_t stream_nr;
      memcpy(&stream_nr, msg, sizeof(int32_t));
      char *filename = msg+sizeof(int32_t);
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      boost::shared_ptr<Data_reader> 
        reader(Data_reader_factory::get_reader(filename));

      set_data_reader(stream_nr, reader);

      MPI_Send(&stream_nr, 1, MPI_INT32, 
               status.MPI_SOURCE, MPI_TAG_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD);

      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
       
  case MPI_TAG_ADD_DATA_READER_TCP2:
    {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;

      int size;
      MPI_Get_elements(&status, MPI_INT64, &size);
      assert(size >= 3); // stream_nr, [ip-addr]+, port
      uint64_t ip_addr[size];
      MPI_Recv(&ip_addr, size, MPI_INT64, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);

      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);
      
      int32_t stream_nr = ip_addr[0];
      uint64_t port = ip_addr[size-1];
      
      boost::shared_ptr<Data_reader> 
        reader(new Data_reader_tcp(ip_addr+1, size-2, port));
      set_data_reader(stream_nr, reader);

      MPI_Send(&stream_nr, 1, MPI_INT32, 
               status.MPI_SOURCE, MPI_TAG_CONNECTION_ESTABLISHED, 
               MPI_COMM_WORLD);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}

bool 
Single_data_reader_controller::eof() {
  if (reader2buffer.get_data_reader() != NULL) {
    if (!reader2buffer.get_data_reader()->eof()) return false;
  }
    
  if (reader2buffer.get_buffer() != NULL) {
    return (reader2buffer.get_buffer()->empty());
  }
  return true;
}

boost::shared_ptr<Single_data_reader_controller::Buffer>
Single_data_reader_controller::buffer() {
  return reader2buffer.get_buffer();
}

void 
Single_data_reader_controller::
set_buffer(boost::shared_ptr<Single_data_reader_controller::Buffer> buff) 
{
  assert(buffer() == NULL);
  assert(false); // No buffering for the time being
  return reader2buffer.set_buffer(buff);
}

boost::shared_ptr<Data_reader>
Single_data_reader_controller::get_data_reader(int reader) {
  assert(reader == 0);
  assert(reader2buffer.get_data_reader() != NULL);
  return reader2buffer.get_data_reader();
}

void 
Single_data_reader_controller::
set_data_reader(int stream_nr, boost::shared_ptr<Data_reader> reader) 
{
  assert(stream_nr == 0);
  assert(reader2buffer.get_data_reader() == NULL);

  reader2buffer.set_data_reader(reader);
  reader2buffer.try_start();
  
  node.hook_added_data_reader(0);
}
