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
#include <time.h>
#include <math.h>

#include "input_node.h"
#include "utils.h"

#include "types.h"
#include "semaphore_buffer.h"

#include "data_reader_buffer.h"
#include "channel_extractor_mark4.h"

#include "mpi_transfer.h"

#include "data_writer_file.h"

Input_node::Input_node(int rank, int station_number, Log_writer *log_writer) :
    Node(rank, log_writer), input_node_ctrl(*this), data_reader_ctrl(*this),
    data_writers_ctrl(*this, MAX_TCP_CONNECTIONS),
input_node_tasklet(NULL), status(WAITING) {
  initialise();
}
Input_node::Input_node(int rank, int station_number) :
    Node(rank), input_node_ctrl(*this), data_reader_ctrl(*this),
    data_writers_ctrl(*this, MAX_TCP_CONNECTIONS),
input_node_tasklet(NULL), status(WAITING) {
  initialise();
}
void Input_node::initialise() {
  get_log_writer()(1) << "Input_node()" << std::endl;
  add_controller(&input_node_ctrl);
  add_controller(&data_reader_ctrl);
  add_controller(&data_writers_ctrl);

  int32_t msg;
  MPI_Send(&msg, 1, MPI_INT32, RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED,
           MPI_COMM_WORLD);

}

void Input_node::set_input_node_parameters(const Input_node_parameters &input_node_param) {
  assert(input_node_tasklet != NULL);
  input_node_tasklet->set_parameters(input_node_param);
}

Input_node::~Input_node() {
  if (input_node_tasklet != NULL)
    delete input_node_tasklet;
}

int32_t Input_node::get_time_stamp() {
  assert(input_node_tasklet != NULL);
  // the time in the tasklet is in micro seconds
  return input_node_tasklet->get_current_time()/1000;
}

int mark4_time=0;
void Input_node::start() {
  while (status != END_NODE) {
    switch (status) {
      case WAITING: { // Wait until we can start sending new data
        // Wait for data_source to become ready
        if (check_and_process_message() == TERMINATE_NODE) {
          status = END_NODE;
          break;
        }
        if (input_node_tasklet != NULL)
          status = WRITING;
        break;
      }
      case WRITING: {
        if (process_all_waiting_messages() == TERMINATE_NODE) {
          DEBUG_MSG("END_INPUT_NODE");
          status = END_NODE;
          break;
        }
        assert(input_node_tasklet != NULL);
        input_node_tasklet->do_task();
        if (!input_node_tasklet->has_work())
          status = WAITING;
        break;
      }
      case END_NODE: {
        assert(false);
      }
    }
  }

  while (!data_writers_ctrl.ready())
    usleep(100000); // .1 second:

  int32_t rank = get_rank();
  MPI_Send(&rank, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_DATASTREAM_EMPTY, MPI_COMM_WORLD);
}

void Input_node::hook_added_data_reader(size_t stream_nr) {
  assert(stream_nr == 0);
  input_node_tasklet =
    get_input_node_tasklet(data_reader_ctrl.get_data_reader(stream_nr));
}

void Input_node::hook_added_data_writer(size_t writer) {}

void Input_node::set_stop_time(int64_t stop_time_) {
  input_node_tasklet->set_stop_time(stop_time_);
}

void Input_node::goto_time(int64_t new_time) {
  assert(input_node_tasklet != NULL);
  input_node_tasklet->goto_time(new_time);
}

void Input_node::add_time_slice(int channel, int stream, int starttime_slice,
                                int stoptime_slice) {
  assert(data_writers_ctrl.get_data_writer(stream) != NULL);
  assert(data_writers_ctrl.get_data_writer(stream)->get_size_dataslice() <= 0);

  assert(input_node_tasklet != NULL);

  input_node_tasklet->add_data_writer(channel,
                                      data_writers_ctrl.get_data_writer(stream),
                                      stoptime_slice-starttime_slice);
}

int Input_node::get_status() {
  return status;
}

void Input_node::set_delay_table(Delay_table_akima &delay_table) {
  assert(input_node_tasklet != NULL);
  input_node_tasklet->set_delay_table(delay_table);
}
