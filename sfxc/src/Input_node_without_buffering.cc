/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id: Input_node.cc 222 2007-05-14 07:22:32Z kruithof $
 *
 */

#include <Input_node.h>

#include <types.h>
#include <Semaphore_buffer.h>

#include <Data_reader_buffer.h>
#include <Channel_extractor_mark4.h>

#include <iostream>
#include <assert.h>
#include <time.h>

#include <MPI_Transfer.h>
#include <errno.h>

#define BUFFER_SIZE 10

Input_node::Input_node(int rank, Log_writer *log_writer) 
  : Node(rank, log_writer), 
    input_node_ctrl(*this),
    data_reader_ctrl(*this),
    data_writers_ctrl(*this),
    channel_extractor(NULL),
    time_stamp(1),
    buffer_size(BUFFER_SIZE),
    status(STOPPED)
{
  initialise();
}
Input_node::Input_node(int rank) 
  : Node(rank), 
    input_node_ctrl(*this),
    data_reader_ctrl(*this),
    data_writers_ctrl(*this),
    channel_extractor(NULL),
    time_stamp(1),
    buffer_size(BUFFER_SIZE),
    nr_input_reader(nr_input_reader),
    status(STOPPED)
{
  initialise();
}
void Input_node::initialise() 
{
  get_log_writer() << "Input_node_without_buffering()" << std::endl;

  // NGHK: These use the global parameters: change
  MPI_Status status;
  MPI_Probe(RANK_MANAGER_NODE, MPI_TAG_CONTROL_PARAM, MPI_COMM_WORLD, &status);
  get_log_writer().MPI(2, print_MPI_TAG(status.MPI_TAG));
  MPI_Transfer mpi_transfer;
  mpi_transfer.receive_general_parameters(status,RunPrms,GenPrms,StaPrms);
  
  add_controller(&input_node_ctrl);
  add_controller(&data_reader_ctrl);
  add_controller(&data_writers_ctrl);

  INT32 msg;
  MPI_Send(&msg, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);
}


Input_node::~Input_node() {
  while (!active_list.empty()) {
    stop_stream(active_list.begin());
  } 
}

void Input_node::set_status() {
  if (status == END_NODE) return;
  
  if (data_reader_ctrl.eof()) {
    status = END_NODE;
  } else if (!active_list.empty()) {
    status = SEND_OUTPUT;
  } else {
    status = STOPPED;
  }
}

void Input_node::start() {
  while (status != END_NODE) {
    switch (status) {
      case STOPPED: {
        // blocking:
        if (check_and_process_message()==TERMINATE_NODE) {
          status = END_NODE;
        }
        break;
      }
      case SEND_OUTPUT: {
        MESSAGE_RESULT msg_result = check_and_process_waiting_message();
        while ((msg_result != NO_MESSAGE) && (status != END_NODE)) {
          if (msg_result == TERMINATE_NODE) {
            status = END_NODE;
            break;
          }
          msg_result = check_and_process_waiting_message();
        }
        
        if (channel_extractor->eof()) {
          status = END_NODE;
          break;
        }
        if (status == SEND_OUTPUT) {
          assert(data_reader_ctrl.buffer() == NULL);
          size_t size = channel_extractor->get_bytes(ch_buffer_size, ch_buffer);
          if (size != ch_buffer_size) {
            status = END_NODE;
            // do write the remaining bytes
          }

          update_active_list();

          assert(!active_list.empty());
          for (std::list<int>::iterator it = active_list.begin();
               it != active_list.end(); it++) {
            assert(data_writers_ctrl.get_data_writer(*it) != NULL);
            data_writers_ctrl.get_data_writer(*it)->put_bytes(size, ch_buffer);
          }
        }
        break;
      }
      case END_NODE: { // For completeness sake, is caught by the while loop
        break;
      }
    }
  }

  while (!data_writers_ctrl.ready()) usleep(100000); // .1 second:

  while (!active_list.empty()) stop_stream(active_list.begin());
  
  INT32 rank = get_rank();
  MPI_Send(&rank, 1, MPI_INT32, 
           RANK_MANAGER_NODE, MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD);
}

void Input_node::set_priority(int stream, UINT64 start, UINT64 stop) {
  start_queue.insert(std::pair<INT64,int>(start, stream));
  // if stop == 0, then output to the stream until the end
  if (stop > 0) stop_queue.insert(std::pair<UINT64,int>(stop, stream));
  
  update_active_list();
}

void Input_node::set_time_stamp(value_type &t) {
  std::cout << "TIME STAMP NOT SET" << std::endl;
}  

void Input_node::update_active_list() {
  // Check start_queue:
  while (!start_queue.empty() &&
         start_queue.begin()->first < get_time_stamp()) {
    int stream = start_queue.begin()->second;
    add_to_active_list(stream);
    start_queue.erase(start_queue.begin());
  }

  // Check stop_queue:
  while (!stop_queue.empty() &&
         stop_queue.begin()->first < get_time_stamp()) {
    bool found = false;
    int stream = stop_queue.begin()->second;
    for (std::list<int>::iterator it = active_list.begin();
         it != active_list.end(); it++) {
      if (*it == stream) {
        it--;
        stop_stream(it);
        found = true;
      }
    }
    assert(found);
    stop_queue.erase(stop_queue.begin());
  }

  set_status();
}  

void Input_node::add_to_active_list(int stream) {
  // Debugging code:
  for (std::list<int>::iterator it = active_list.begin();
       it != active_list.end(); it++) {
    assert(*it != stream);
  }
  
  active_list.push_back(stream);
  assert(data_writers_ctrl.buffer(stream) == NULL);
  data_writers_ctrl.set_buffer(stream, new Buffer(buffer_size));
}

void Input_node::remove_from_active_list(int stream) {
  bool found = false;
  for (std::list<int>::iterator it = active_list.begin();
       it != active_list.end(); it++) {
    if (*it == stream) {
      assert(!found);
      if (it == active_list.begin()) {
        active_list.erase(it);
        it = active_list.begin();
      } else {
        std::list<int>::iterator it_del = it;
        it--;
        active_list.erase(it_del);
      }
      found = true;
    }
  }
  assert(found);
}

void Input_node::stop_stream(const std::list<int>::iterator &stream_it) {
  if (data_writers_ctrl.get_rank_node_reader(*stream_it) >= 0) {
    UINT64 msg[] =
      {data_writers_ctrl.get_stream_number_reader(*stream_it),
       data_writers_ctrl.get_data_writer(*stream_it)->data_counter()};

    MPI_Send(&msg, 2, MPI_UINT64, 
             data_writers_ctrl.get_rank_node_reader(*stream_it),
             MPI_TAG_OUTPUT_STREAM_TIME_SLICE_FINISHED, 
             MPI_COMM_WORLD);
  }
  active_list.erase(stream_it);
}

void Input_node::hook_added_data_reader(size_t stream_nr) {
  assert(channel_extractor == NULL);
  assert(data_reader_ctrl.get_data_reader() != NULL);
  channel_extractor = 
    new Channel_extractor_mark4(*data_reader_ctrl.get_data_reader(), 
                                StaPrms[nr_input_reader], GenPrms.get_rndhdr());
}

void Input_node::hook_added_data_writer(size_t writer) {
}