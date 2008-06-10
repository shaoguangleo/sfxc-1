/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *
 * $Id$
 *
 */

#ifndef CHANNEL_EXTRACTOR_TASKLET_H_
#define CHANNEL_EXTRACTOR_TASKLET_H_

#include <vector>

#include "utils.h"
#include "tasklet/tasklet.h"
#include "input_node_types.h"
#include "control_parameters.h"

#include "channel_extractor_interface.h"

#ifdef RUNTIME_STATISTIC
#include "monitor.h"
#endif //RUNTIME_STATISTIC


#define MAX_SUBBANDS 16

/**
 * The channel extractor gets a chunk of data and outputs the dechannelized data
 **/
class Channel_extractor_tasklet : public Tasklet {
public:
  typedef Input_node_types                     Types;

  /// Data type of the input buffer
  typedef Types::Mark5_buffer                  Input_buffer;
  /// Data type of a data element in the input buffer
  typedef Input_buffer::value_type             Input_buffer_element;
  /// Pointer to an Input_buffer
  typedef boost::shared_ptr<Input_buffer>      Input_buffer_ptr;

  /// Output memory pool for the dechannelized data
  typedef Types::Channel_memory_pool           Output_memory_pool;
  /// Queue for sending out the dechannalized data
  typedef Types::Channel_buffer                Output_buffer;
  /// A data element of the Output_buffer
  typedef Types::Channel_buffer_element        Output_buffer_element;
  /// A pointer to the Output_buffer
  typedef boost::shared_ptr<Output_buffer>     Output_buffer_ptr;

  /**
   * Constructor
   * \param samples_per_block Number of input words to process
   * \param N                 Number of bytes per input word
   **/
  Channel_extractor_tasklet(int samples_per_block, int N);

  virtual ~Channel_extractor_tasklet();

  /// For tasklet

  /// Process one piece of data
  void do_task();
  /// Check if we can process data
  bool has_work();
  const char *name() {
    return "Channel_extractor";
  }

  /// Set the input queue
  void connect_to(Input_buffer_ptr new_input_buffer);

  /** Get the output
   * \param stream The channel number for which you want the stream (0-based)
   **/
  Output_buffer_ptr get_output_buffer(size_t stream);

  /**
   * Initialises the channel extractor. The track_positions describe the layout
   * of the channels on the tracks. For channel i you need tracks 
   * track_positions[i]. 
   **/
  void set_parameters(const Input_node_parameters &input_node_param,
                      const std::vector< std::vector<int> > &track_positions);

private:
  /// Queue containing input data
  Input_buffer_ptr                input_buffer_;
  /// Memory pool containing data chunks of dechannelized data
  Output_memory_pool              output_memory_pool_;
  /// List of the output queues
  std::vector<Output_buffer_ptr>  output_buffers_;

  /// Actual channel extractor
  Channel_extractor_interface     *ch_extractor;

  // Cached values
  size_t n_subbands;      //< Number of subbands
  size_t fan_out;         //< Number of output bits per input sample
  size_t bits_per_sample; //< Number of bits per output sample

  // Size of one input word in bytes (#tracks/8)
  const int N;                 ///< Number of bytes per input word
  const int samples_per_block; ///< Number input words per input chunk of data.

#ifdef RUNTIME_STATISTIC

  QOS_MonitorSpeed monitor_;
#endif //RUNTIME_STATISTIC
};

#endif /*CHANNEL_EXTRACTOR_TASKLET_H_*/