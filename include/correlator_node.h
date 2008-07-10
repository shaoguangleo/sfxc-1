/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef CORRELATOR_NODE_H
#define CORRELATOR_NODE_H
#include <queue>
#include "node.h"
#include "multiple_data_readers_controller.h"
#include "single_data_writer_controller.h"
#include "control_parameters.h"
#include "correlator_node_data_reader_tasklet.h"
#include "log_writer_mpi.h"
#include "correlation_core.h"
#include "delay_correction.h"
#include <tasklet/tasklet_manager.h>
#include "timer.h"

#include "monitor.h"
#include "eventor_poll.h"

// Declare the correlator controller:
class Correlator_node;

/**
 * Correlator_node_controller processes specific signals for the Correlator node.
 **/
class Correlator_node_controller : public Controller {
public:
  Correlator_node_controller(Correlator_node &node);
  ~Correlator_node_controller();

  Process_event_status process_event(MPI_Status &status);

private:
  Correlator_node &node;
};

/**
 * A correlate node will initialize the correlation process and connect
 * to the output node. It can receive messages from a data node asking to
 * open an input connection and from the controller node to process a
 * time slice. After the slice is processed the node will send a message
 * to the controller node saying it is available for a next job.
 *
 * \ingroup Node
 **/
class Correlator_node : public Node {
public:
  typedef Correlator_node                              Self;
  typedef Memory_pool_vector_element<char>             output_value_type;
  typedef boost::shared_ptr<Correlator_node_data_reader_tasklet>
  Bit_sample_reader_ptr;

  typedef boost::shared_ptr<Delay_correction>         Delay_correction_ptr;

  bool has_requested;

  /// The states of the correlator_node.
  enum Status {
    // Initialise the Correlate node
    STOPPED=0,
    // The node is correlating
    CORRELATING,
    END_CORRELATING
  };

  Correlator_node(int rank, int nr_corr_node);
  ~Correlator_node();

  /// The the main_loop of the correlator node.
  void start();

  /// Terminate the main_loop.
  void terminate();

  /// Callback function for adding a data_reader:
  void hook_added_data_reader(size_t reader);

  /// Callback function for adding a data_writer:
  void hook_added_data_writer(size_t writer);

  void add_delay_table(int sn, Delay_table_akima &table);

  void output_node_set_timeslice(int slice_nr, int slice_offset, int n_slices,
                                 int stream_nr, int bytes);

  void receive_parameters(const Correlation_parameters &parameters);

  void set_parameters();

  int get_correlate_node_number();


class Reader_thread : public Thread {
    std::vector< Bit_sample_reader_ptr >        bit_sample_readers_;

    struct job {
      int number_ffts_in_integration;
      int bits_per_sample;
      int number_channels;
      int station_streams_size;
    };

    Threadsafe_queue<struct job> queue_;
    int num_reading_;
    /// Time spend in waiting for new slice
    Timer timer_waiting_;

    /// Time spend in waiting for data to arrive
    Timer timer_breading_;

    /// Time spend in really reading the data
    Timer timer_reading_;

  public:
    std::vector< Bit_sample_reader_ptr >& bit_sample_readers() {
      return bit_sample_readers_;
    }

    ~Reader_thread() {
      double total_duration = timer_waiting_.measured_time()+timer_reading_.measured_time();
      double ratio1 = ((100.0*timer_waiting_.measured_time())/total_duration);
      double ratio2 = ((100.0*timer_reading_.measured_time())/total_duration);
      PROGRESS_MSG( "reading ratio:("<< ratio1 <<"%, "<< ratio2 <<"%)" );
    }

  class Listener : public FdEventListener {
      Bit_sample_reader_ptr& reader_;

    public:
      Listener(Bit_sample_reader_ptr& ptr ) : reader_(ptr) {};

      void on_event(short event) {
        if ( reader_->has_work() )
          reader_->do_task();
      };

      void on_error(short event) {};
    };

    Eventor_poll eventsrc_;

    void do_execute() {
      for (unsigned int i=0;i<bit_sample_readers_.size();i++) {
        eventsrc_.add_listener( POLLIN,
                                bit_sample_readers_[i]->get_fd(),
                                new Listener( bit_sample_readers_[i] ) );
      }

      //eventsrc_.randomize();

      isrunning_ = true;
      DEBUG_MSG("reading thread started !");
      try {
        while ( isrunning_ ) {
          if ( num_reading_ == 0 ) {
            timer_waiting_.resume();
            fetch_new_time_slice();
            timer_waiting_.stop();
          } else {
            timer_reading_.resume();
            /// Wait something happens.
            eventsrc_.wait_until_any_event();
            num_reading_ = 0;
            for ( unsigned int i= 0;i<bit_sample_readers_.size();i++) {
              num_reading_+=bit_sample_readers_[i]->data_to_read();
            }
            timer_reading_.stop();
          }
        }
      } catch (QueueClosedException& exp) {
        std::cout << "The queue is closed !" << std::endl;
      }
    }

    void stop() {
      isrunning_ = false;
      queue_.close();
    }

    void fetch_new_time_slice() {
      // Immediately start prefetching the data:
      struct job jb = queue_.front();
      DEBUG_MSG("I HAVE NOW SOME DATA :" << jb.station_streams_size);

      num_reading_=0;
      for (size_t i=0; i<bit_sample_readers_.size(); i++) {
        SFXC_ASSERT(bit_sample_readers_[i] !=
                    Bit_sample_reader_ptr());
        if (i < jb.station_streams_size ) {
          DEBUG_MSG("CONFIGURING THE READER ! :" << jb.number_ffts_in_integration);

          bit_sample_readers_[i]->set_parameters(
            jb.number_ffts_in_integration,
            jb.bits_per_sample,
            jb.number_channels
          );
          num_reading_++;
        }
      }
      queue_.pop();
    }


    /// This function add a new timeslice to read...
    void add_time_slice_to_read(const Correlation_parameters& parameters) {
      struct job jb;
      jb.number_ffts_in_integration =
        Control_parameters::nr_ffts_per_integration_slice
        (parameters.integration_time,
         parameters.sample_rate,
         parameters.number_channels);
      jb.bits_per_sample = parameters.bits_per_sample;
      jb.number_channels = parameters.number_channels;
      jb.station_streams_size = parameters.station_streams.size();

      DEBUG_MSG("Add A Time slice:" << jb.station_streams_size );
      queue_.push(jb);

    }
  };

private:
  Reader_thread reader_thread_;
  ThreadPool threadpool_;
  void start_threads();
  void stop_threads();

  /// Main "usefull" function in which the real correlation computation is
  /// done.
  void correlate();
  void main_loop();

  //std::vector< Bit_sample_reader_ptr>         bit_sample_readers;

  Correlator_node_controller       correlator_node_ctrl;

  /// The correlator node is connected to each of the input nodes.
  Multiple_data_readers_controller data_readers_ctrl;

  Single_data_writer_controller    data_writer_ctrl;

  /// State variables:
  Status status;

  /// Number of the correlator node
  int nr_corr_node;

  std::vector< Delay_correction_ptr >         delay_modules;
  Correlation_core                            correlation_core;

  int n_integration_slice_in_time_slice;

  std::queue<Correlation_parameters>          integration_slices_queue;

  Timer bit_sample_reader_timer_, bits_to_float_timer_, delay_timer_, correlation_timer_;

  bool isinitialized_;

#ifdef RUNTIME_STATISTIC
  QOS_MonitorSpeed reader_state_;
  QOS_MonitorSpeed delaycorrection_state_;
  QOS_MonitorSpeed correlation_state_;
  QOS_MonitorSpeed dotask_state_;
#endif //RUNTIME_STATISTIC
};

#endif // CORRELATOR_NODE_H
