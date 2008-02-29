/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */


#include <iostream>
#include <assert.h>

// Output_node_controller is defined in Output_node.h:
#include "output_node.h"

#include "data_writer_file.h"
#include "data_reader_tcp.h"
#include "tcp_connection.h"
#include "queue_buffer.h"

Output_node_controller::Output_node_controller(Output_node &node)
  : Controller(node), node(node) {
    
}

Controller::Process_event_status
Output_node_controller::process_event(MPI_Status &status) {
  MPI_Status status2;
  switch (status.MPI_TAG) {
  case MPI_TAG_OUTPUT_NODE_GLOBAL_HEADER:
  {
    int size;
    MPI_Get_elements(&status, MPI_CHAR, &size);
    assert(size == sizeof(Output_header_global));
    Output_header_global global_header;
    MPI_Recv((char *)&global_header, size, MPI_CHAR, status.MPI_SOURCE,
             status.MPI_TAG, MPI_COMM_WORLD, &status2);
    node.write_global_header(global_header);
    
    return PROCESS_EVENT_STATUS_SUCCEEDED;
  }
  case MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY:
    {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      int32_t weight[3]; // stream, slicenr, size (in bytes)
      MPI_Recv(&weight, 3, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      
      assert(status.MPI_SOURCE == status2.MPI_SOURCE);
      assert(status.MPI_TAG == status2.MPI_TAG);

      // Create an output buffer:
      node.set_weight_of_input_stream(weight[0], weight[1], weight[2]);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  case MPI_TAG_OUTPUT_NODE_CORRELATION_READY:
    {
      get_log_writer()(3) << print_MPI_TAG(status.MPI_TAG) << std::endl;
      int32_t nr_of_time_slices;
      MPI_Recv(&nr_of_time_slices, 1, MPI_INT32, status.MPI_SOURCE,
               status.MPI_TAG, MPI_COMM_WORLD, &status2);
      assert(nr_of_time_slices >= 0);
      node.set_number_of_time_slices(nr_of_time_slices);
      
      return PROCESS_EVENT_STATUS_SUCCEEDED;
    }
  }
  return PROCESS_EVENT_STATUS_UNKNOWN;
}