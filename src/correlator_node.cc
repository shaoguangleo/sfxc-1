/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Aard Keimpema <Keimpema@JIVE.nl>, 2008
 *
 * $Id$
 *
 */

#include "correlator_node.h"

#include "data_reader_buffer.h"
#include "data_writer.h"
#include "utils.h"
#include "output_header.h"
#include "delay_correction_swapped.h"
#include "delay_correction_default.h"


Correlator_node::Correlator_node(int rank, int nr_corr_node, int swap_)
    : Node(rank),
    correlator_node_ctrl(*this),
    data_readers_ctrl(*this),
    data_writer_ctrl(*this),
    status(STOPPED),
    isinitialized_(false),
    nr_corr_node(nr_corr_node), 
    correlation_core(swap_), swap(swap_) {
  get_log_writer()(1) << "Correlator_node(" << nr_corr_node << ")" << std::endl;

  add_controller(&correlator_node_ctrl);
  add_controller(&data_readers_ctrl);
  add_controller(&data_writer_ctrl);


  int32_t msg;
  MPI_Send(&msg, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_NODE_INITIALISED, MPI_COMM_WORLD);

  MPI_Send(&nr_corr_node, 1, MPI_INT32,
           RANK_MANAGER_NODE, MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED,
           MPI_COMM_WORLD);

  has_requested=true;


#ifdef RUNTIME_STATISTIC

  std::stringstream inputid;
  std::stringstream compid;
  std::stringstream monid;
  std::stringstream tt;

  inputid << "correlationnode" << RANK_OF_NODE;

  compid << inputid.str() << "_dotask";
  monid << compid.str() << "_monitor_state";
  dotask_state_.init(monid.str());
  dotask_state_.add_property(inputid.str(), "is_a", "correlationnode");
  dotask_state_.add_property(inputid.str(), "has", compid.str() );
  dotask_state_.add_property(compid.str(), "is_a", "correlationnode_dotaskloop");
  dotask_state_.add_property(compid.str(), "has", monid.str() );
  tt.str(monid.str());

  compid.str("");
  monid.str("");
  compid << inputid.str() << "_reader";
  monid << compid.str() << "_monitor_state";
  reader_state_.init(monid.str());
  reader_state_.add_property(inputid.str(), "is_a", "correlationnode");
  reader_state_.add_property(inputid.str(), "has", compid.str() );
  reader_state_.add_property(compid.str(), "is_a", "reader_component");
  reader_state_.add_property(compid.str(), "has", monid.str() );
  dotask_state_.add_property(tt.str(), "contains", monid.str() );

  compid.str("");
  monid.str("");
  compid << inputid.str() << "_delaycorrection";
  monid << compid.str() << "_monitor_state";
  delaycorrection_state_.init(monid.str());
  delaycorrection_state_.add_property(inputid.str(), "is_a", "correlationnode");
  delaycorrection_state_.add_property(inputid.str(), "has", compid.str() );
  delaycorrection_state_.add_property(compid.str(), "is_a", "delaycorrection_component");
  delaycorrection_state_.add_property(compid.str(), "has", monid.str() );
  dotask_state_.add_property(tt.str(), "contains", monid.str() );

  compid.str("");
  monid.str("");
  compid << inputid.str() << "_integration";
  monid << compid.str() << "_monitor_state";
  correlation_state_.init(monid.str());
  correlation_state_.add_property(inputid.str(), "is_a", "correlationnode");
  correlation_state_.add_property(inputid.str(), "has", compid.str() );
  correlation_state_.add_property(compid.str(), "is_a", "correlation_component");
  correlation_state_.add_property(compid.str(), "has", monid.str() );
  correlation_state_.add_property(tt.str(), "contains", monid.str() );
#endif //RUNTIME_STATISTIC

}

Correlator_node::~Correlator_node() {
#if PRINT_TIMER
  PROGRESS_MSG("Time bit_sample_reader:  " << bit_sample_reader_timer_.measured_time());
  PROGRESS_MSG("Time bits2float:  " << bits_to_float_timer_.measured_time());
  PROGRESS_MSG("Time delay:       " << delay_timer_.measured_time());
  PROGRESS_MSG("Time correlation: " << correlation_timer_.measured_time());
#endif
}

void Correlator_node::start_threads() {
  threadpool_.register_thread( reader_thread_.start() );
}

void Correlator_node::stop_threads() {
  reader_thread_.stop();

  /// We wait the termination of the threads
  threadpool_.wait_for_all_termination();
}

void Correlator_node::start() {
  /// We enter the main loop of the coorelator node.
  ///DEBUG_MSG("START MAIN LOOp !");
  main_loop();
}

void Correlator_node::terminate() {
  DEBUG_MSG("Correlator node received terminate signal.");
  status = END_CORRELATING;
}

void Correlator_node::main_loop() {
  while ( status != END_CORRELATING ) {
    switch (status) {
    case STOPPED: {
        /// We wait for a message
        check_and_process_message();
        // blocking:
        break;
      }
    case CORRELATING: {
        process_all_waiting_messages();

        correlate();
        if (!has_requested && correlation_core.almost_finished()) {
          //if (n_integration_slice_in_time_slice==1)
          //{
          ///DEBUG_MSG("TIME TO GET NEW DATA !");
          // Notify manager node:
          int32_t msg = get_correlate_node_number();
          MPI_Send(&msg, 1, MPI_INT32, RANK_MANAGER_NODE,
                   MPI_TAG_CORRELATION_OF_TIME_SLICE_ENDED,
                   MPI_COMM_WORLD);

          has_requested = true;
          //}
        }
        if (correlation_core.finished()) {
          ///DEBUG_MSG("CORRELATION CORE FINISHED !" << n_integration_slice_in_time_slice);

          n_integration_slice_in_time_slice--;
          if (n_integration_slice_in_time_slice==0) {
            // Notify manager node:
            status = STOPPED;
            // Try initialising a new integration slice
            set_parameters();
          }
        }
        break;
      }
    case END_CORRELATING:
      break;

    }
  }

  stop_threads();
}

void Correlator_node::add_delay_table(int sn, Delay_table_akima &table) {
  SFXC_ASSERT((size_t)sn < delay_modules.size());
  //  SFXC_ASSERT(integer_delay_modules[sn] != Integer_delay_correction_ptr());
  SFXC_ASSERT(delay_modules[sn] != Delay_correction_ptr());
  //  integer_delay_modules[sn]->set_delay_table(table);
  delay_modules[sn]->set_delay_table(table);
}

void Correlator_node::hook_added_data_reader(size_t stream_nr) {
  // create the bit sample reader tasklet
  if (reader_thread_.bit_sample_readers().size() <= stream_nr) {
    reader_thread_.bit_sample_readers().resize(stream_nr+1, Bit_sample_reader_ptr());
  }
  reader_thread_.bit_sample_readers()[stream_nr] =
    Bit_sample_reader_ptr(new Correlator_node_data_reader_tasklet());
  reader_thread_.bit_sample_readers()[stream_nr]->connect_to(data_readers_ctrl.get_data_reader(stream_nr));
  
  { // create the delay modules
    if (delay_modules.size() <= stream_nr) {
      delay_modules.resize(stream_nr+1,
                           boost::shared_ptr<Delay_correction_base>());
    }
    if(swap==0){
      delay_modules[stream_nr] = Delay_correction_ptr(new Delay_correction_default());
      Delay_correction_default *mod = (Delay_correction_default *)delay_modules[stream_nr].get();
      // Connect the delay_correction to the bits2float_converter
      mod->connect_to(reader_thread_.bit_sample_readers()[stream_nr]->get_output_buffer());
      correlation_core.connect_to(stream_nr,mod->get_output_buffer());
    }else{
      delay_modules[stream_nr] = Delay_correction_ptr(new Delay_correction_swapped());
      Delay_correction_swapped *mod = (Delay_correction_swapped *)delay_modules[stream_nr].get();      
      // Connect the delay_correction to the bits2float_converter
      mod->connect_to(reader_thread_.bit_sample_readers()[stream_nr]->get_output_buffer());
      correlation_core.connect_to(stream_nr,mod->get_output_buffer());
    }
  }

}

void Correlator_node::hook_added_data_writer(size_t i) {
  SFXC_ASSERT(i == 0);

  correlation_core.set_data_writer(data_writer_ctrl.get_data_writer(0));
}

int Correlator_node::get_correlate_node_number() {
  return nr_corr_node;
}

void Correlator_node::correlate() {
  RT_STAT( dotask_state_.begin_measure() );

  delay_timer_.resume();
  for (size_t i=0; i<delay_modules.size(); i++) {
    if (delay_modules[i] != Delay_correction_ptr()) {
      if (delay_modules[i]->has_work()) {
        RT_STAT( delaycorrection_state_.begin_measure() );

        delay_modules[i]->do_task();
        RT_STAT( delaycorrection_state_.end_measure(1) );

      }
    }
  }
  delay_timer_.stop();

  correlation_timer_.resume();
  if (correlation_core.has_work()) {
    RT_STAT( correlation_state_.begin_measure() );

    correlation_core.do_task();
    RT_STAT( correlation_state_.end_measure(1) );


  }

  correlation_timer_.stop();

  RT_STAT( dotask_state_.end_measure(1) );
}

void
Correlator_node::receive_parameters(const Correlation_parameters &parameters) {
  integration_slices_queue.push(parameters);

  /// We add the new timeslice to the readers.
  reader_thread_.add_time_slice_to_read(parameters);

  if (status == STOPPED)
    set_parameters();

}
void
Correlator_node::set_parameters() {
  SFXC_ASSERT(status == STOPPED);

  if ( !isinitialized_ ) {
    ///DEBUG_MSG("START THE THREADS !");
    isinitialized_ = true;
    start_threads();
  }

  if (integration_slices_queue.empty())
    return;

  const Correlation_parameters &parameters =
    integration_slices_queue.front();

  int size_input_slice =
    Control_parameters::nr_bytes_per_integration_slice_input_node_to_correlator_node
    (parameters.integration_time,
     parameters.sample_rate,
     parameters.bits_per_sample,
     parameters.number_channels);

  SFXC_ASSERT(((parameters.stop_time-parameters.start_time) /
               parameters.integration_time) == 1);

  SFXC_ASSERT(size_input_slice > 0);

  for (size_t i=0; i<delay_modules.size(); i++) {
    if (delay_modules[i] != Delay_correction_ptr()) {
      delay_modules[i]->set_parameters(parameters);
    }
  }
  correlation_core.set_parameters(parameters, get_correlate_node_number());

  has_requested=false;
  status = CORRELATING;

  n_integration_slice_in_time_slice =
    (parameters.stop_time-parameters.start_time) / parameters.integration_time;
  // set the output stream
  int nBaselines = correlation_core.number_of_baselines();
  int size_of_one_baseline = sizeof(fftwf_complex)*
                             (parameters.number_channels*PADDING/2+1);
  int size_uvw = correlation_core.uvw_tables.size()*sizeof(Output_uvw_coordinates);

  output_node_set_timeslice(parameters.slice_nr,
                            parameters.slice_offset,
                            n_integration_slice_in_time_slice,
                            get_correlate_node_number(),
                            sizeof(Output_header_timeslice) + size_uvw +
                            ( nBaselines *
                              (size_of_one_baseline +
                               sizeof(Output_header_baseline) ) ) );

  integration_slices_queue.pop();
}

void
Correlator_node::
output_node_set_timeslice(int slice_nr, int slice_offset, int n_slices,
                          int stream_nr, int bytes) {
  correlation_core.data_writer()->set_size_dataslice(bytes*n_slices);
  int32_t msg_output_node[] = {stream_nr, slice_nr, bytes};
  for (int i=0; i<n_slices; i++) {
    MPI_Send(&msg_output_node, 3, MPI_INT32,
             RANK_OUTPUT_NODE,
             MPI_TAG_OUTPUT_STREAM_SLICE_SET_PRIORITY,
             MPI_COMM_WORLD);
    msg_output_node[1] += slice_offset;
  }
}
