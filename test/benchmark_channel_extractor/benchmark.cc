/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)

 * Copyright (c) 2007 University of Amsterdam (Netherlands)

 * All rights reserved.

 *

 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007

 *            Damien Marchal <dmarchal@science.uva.nl>, 2007

 *

 *

 * This file is part of:

 *   - sfxc/SCARIe

 * This file contains:

 *   - the implementation of the benchmark (and regression test)
 *     class for the channel_extractor.
 *

 */
#include "benchmark.h"
#include "channel_extractor_brute_force.h"

#include <iostream>
#include <vector>
#include <assert.h>

#include "utils_bench.h"
#include "timer.h"

Benchmark::Benchmark(Channel_extractor_interface &channel_extractor_)
    : channel_extractor(channel_extractor_) {}

bool Benchmark::test() {
  int element_size = 16;
  bool ok = true;
  bool early_exit = true;
  ORDER type[]={CHANNEL_ORDER, FAN_OUT_ORDER, EOL_ORDER};
  int idx=0;
  while ( type[idx] != EOL_ORDER ) {
    std::cout << ".";
    std::cout.flush();
    ok &= do_test(4,4,type[idx], element_size);
    if (early_exit && !ok) exit(0);

    std::cout << ".";
    std::cout.flush();
    ok &= do_test(8,4,type[idx], element_size);
    if (early_exit && !ok) exit(0);

    std::cout << ".";
    std::cout.flush();
    ok &= do_test(8,2,type[idx], element_size);
    if (early_exit && !ok) exit(0);

    std::cout << ".";
    std::cout.flush();
    ok &= do_test(8,8,type[idx], element_size);
    if (early_exit && !ok) exit(0);

    std::cout << ".";
    std::cout.flush();
    ok &= do_test(16,4,type[idx], element_size);
    if (early_exit && !ok) exit(0);

    idx++;
  }
  return ok;
}


double Benchmark::benchmark(int datasize, int repeat) {
  ORDER type[]={CHANNEL_ORDER, FAN_OUT_ORDER, EOL_ORDER};
  int idx=0;
  while ( type[idx] != EOL_ORDER ) {
    do_benchmark(4,4, type[idx], datasize, repeat);
    do_benchmark(8,2, type[idx], datasize, repeat);
    do_benchmark(8,4, type[idx], datasize, repeat);
    do_benchmark(8,8, type[idx], datasize, repeat);
    do_benchmark(16,4, type[idx], datasize, repeat);
    idx++;
  }
  return 1;
}


void      print_compare(unsigned char in1[],
                        unsigned char in2[],
                        unsigned char rein[],
                        int start_in_two,
                        int total_num_words
                       ) {
  std::cout << std::endl;
  int begin = total_num_words-start_in_two+1;
  for (unsigned int i=0;i<start_in_two;i++) {
    std::cout << i << ".a: ";
    print_hex(std::cout, in1[i]);
    std::cout << " vs ";
    print_hex(std::cout, rein[begin]);
    std::cout << std::endl;
    begin++;
  }

  for (unsigned int i=0;i<total_num_words-start_in_two+1;i++) {
    std::cout << i << ".b: ";
    print_hex(std::cout, in2[i+begin]);
    std::cout << " vs ";
    print_hex(std::cout, rein[i+begin]);
    std::cout << std::endl;
  }

}


bool Benchmark::do_test(int n_channels, int fan_out,
                        ORDER ordering,
                        int n_input_samples_to_process) {
  bool result = true;
  assert((fan_out*n_channels) % 8 == 0);
  // Size of one input data sample in bytes
  const int size_input_word = fan_out*n_channels/8;
  Channel_extractor_brute_force channel_extractor_brute;

  // Initialise the track positions
  std::vector< std::vector<int> > track_positions;
  initialise(n_channels, fan_out, ordering, track_positions);


  int bytes_per_sample = 8/track_positions[0].size()*size_input_word;
  int word_per_sample = 8/track_positions[0].size();

  // Initialise the input data
  const int input_size = size_input_word*(n_input_samples_to_process+1);
  unsigned char in_data1[input_size],in_data2[input_size];

  // Allocate the output buffers
  std::vector<unsigned char *> output, output_brute;
  const int n_output_bytes_per_channel = fan_out*n_input_samples_to_process/8;
  output.resize(n_channels);
  output_brute.resize(n_channels);
  for (int i=0; i<n_channels; i++) {
    output[i] = new unsigned char[n_output_bytes_per_channel];
    output_brute[i] = new unsigned char[n_output_bytes_per_channel];
  }

  // Initialise the extractors
  channel_extractor.initialise(track_positions,
                               size_input_word,
                               n_input_samples_to_process);
  channel_extractor_brute.initialise(track_positions,
                                     size_input_word,
                                     n_input_samples_to_process);

  assert(input_size == size_input_word*(n_input_samples_to_process+1));

  for (int offset=0; offset<1; offset++) {
    // randomize input data:
    for (int i=0; i<input_size; i++) {
      if ( i < 256 )
        in_data1[i] = i;
      else in_data1[i]=random();
      in_data2[i] = i;
      random();

    }

    // Brute force for the reference
    randomize_buffers(output_brute, n_output_bytes_per_channel);
    channel_extractor_brute.extract(&in_data1[0],
                                    &output_brute[0]);

    //std::cout << "Offset: " << offset << std::endl;

    for (int offset_in_input_samples=0; offset_in_input_samples<1; offset_in_input_samples++) {
      // recompute the output
      /*
           std::cout << "Offset_in_input_samples: " << offset_in_input_samples << std::endl;
           for (int ii=0;ii< input_size;ii++)
             {
               std::cout << ii <<": a(" << (int)in_data1[ii] << ") b(" << (int)in_data2[ii] << ")" << std::endl;
             }
           std::cout << std::endl;
      */

      randomize_buffers(output, n_output_bytes_per_channel);
      channel_extractor.extract(&in_data1[0],
                                &output[0]);

      // print_output_buffers(output, n_output_bytes_per_channel);





      // check the result:
      result &= check_output_buffers(in_data1,
                                     in_data2,
                                     &output[0],
                                     &output_brute[0],
                                     n_channels,
                                     n_output_bytes_per_channel,
                                     size_input_word,
                                     n_input_samples_to_process+1-offset_in_input_samples,
                                     offset_in_input_samples,
                                     offset,
                                     input_size,
                                     track_positions);

      if (!result) {
        for (int i=0; i<n_channels; i++) {
          delete[] output[i];
          delete[] output_brute[i];
        }
        return result;
      }

      { // shift all samples one input position:
        for (int i=input_size-1; i>=bytes_per_sample; i--)
          in_data2[i] = in_data2[i-bytes_per_sample];
        for (int i=bytes_per_sample-1; i>=0; i--)
          in_data2[i] = in_data1[input_size-bytes_per_sample+i];
        for (int i=input_size-1; i>=bytes_per_sample; i--)
          in_data1[i] = in_data1[i-bytes_per_sample];
      }
    }
  }

  // Clear up the buffers and return
  for (int i=0; i<n_channels; i++) {
    delete[] output[i];
    delete[] output_brute[i];
  }
  return result;
}

void Benchmark::initialise(int n_channels,
                           int fan_out,
                           ORDER ordering,
                           std::vector<std::vector<int> >& track_positions) {
  track_positions.resize(n_channels);
  for (int i=0; i<n_channels; i++)
    track_positions[i].resize(fan_out);


  if ( n_channels == 8 && fan_out == 4 ) {
    int Channels[8][4]={
                         {0, 4, 2, 6},
                         {8, 12, 10, 14},
                         {16, 20, 18, 22},
                         {24, 28, 26, 30},
                         {1, 5, 3, 7},
                         {9, 13, 11, 15},
                         {17, 21, 19, 23},
                         {25, 29, 27, 31}};


    for (int i=0; i<n_channels; i++) {
      for (int j=0; j<fan_out; j++) {
        track_positions[i][j] = Channels[i][j];
      }
    }
    return;
  }

  switch (ordering) {
  case CHANNEL_ORDER: {
      int track=0;
      for (int i=0; i<n_channels; i++) {
        for (int j=0; j<fan_out; j++) {
          track_positions[i][j] = track;
          track++;
        }
      }
      break;
    }
  case FAN_OUT_ORDER: {
      int track=0;
      for (int j=0; j<fan_out; j++) {
        for (int i=0; i<n_channels; i++) {
          track_positions[i][j] = track;
          track++;
        }
      }
      break;
    }
  case RANDOM_ORDER: {


      assert(false);
    }
  }
}

void Benchmark::do_benchmark(int n_channels, int fan_out,
                             ORDER ordering,
                             int n_input_samples_to_process,
                             int repeat) {
  bool result = true;
  assert((fan_out*n_channels) % 8 == 0);
  // Size of one input data sample in bytes
  const int size_input_word = fan_out*n_channels/8;
  Channel_extractor_brute_force channel_extractor_brute;

  std::cout << "\nbenchmarking with parameters: " << std::endl;
  std::cout << "\t\t n_channels: " << n_channels << std::endl;
  std::cout << "\t\t    fan_out: " << fan_out << std::endl;
  std::cout << "\t\t      ORDER: " << ordering << std::endl;
  std::cout << "\t\t    samples: " << n_input_samples_to_process << std::endl;
  std::cout << "\t\t     repeat: " << repeat << std::endl;

  // Initialise the track positions
  std::vector< std::vector<int> > track_positions;
  initialise(n_channels, fan_out, ordering, track_positions);

  // Initialise the input data
  const int input_size = size_input_word*(n_input_samples_to_process+1);
  const int total_size = input_size*repeat;
  std::cout << "\t\t  data size: "  << total_size/(1024*1024) << " MB";

  unsigned char in_data1[input_size];

  // Allocate the output buffers
  std::vector<unsigned char *> output, output_brute;
  const int n_output_bytes_per_channel = fan_out*n_input_samples_to_process/8;
  output.resize(n_channels);
  output_brute.resize(n_channels);
  for (int i=0; i<n_channels; i++) {
    output[i] = new unsigned char[n_output_bytes_per_channel];
    output_brute[i] = new unsigned char[n_output_bytes_per_channel];
  }

  // Initialise the extractors
  channel_extractor.initialise(track_positions,
                               size_input_word,
                               n_input_samples_to_process);

  channel_extractor_brute.initialise(track_positions,
                                     size_input_word,
                                     n_input_samples_to_process);


  randomize_buffers(output_brute, n_output_bytes_per_channel);
  channel_extractor_brute.extract(&in_data1[0],
                                  &output_brute[0]);
  randomize_buffers(output, n_output_bytes_per_channel);

  Timer timer;
  timer.resume();
  for (unsigned int i=0;i<repeat;i++) {
    channel_extractor.extract(&in_data1[0],
                              &output[0]);

  }
  double t = timer.measured_time();
  std::cout << "\n\t\t   timing: "  << total_size/(1024*1024) << " MB in "<< t << "sec ";
  std::cout << "Speed: " << total_size/(1024*1024)/t << "MBps" << std::endl;
  timer.stop();


  for (int i=0; i<n_channels; i++) {
    delete[] output[i];
    delete[] output_brute[i];
  }
}

bool Benchmark::check_output_buffers(unsigned char in1[],
                                     unsigned char in2[],
                                     unsigned char * out1[],
                                     unsigned char * out2[],
                                     int n_channels,
                                     int output_bytes_per_channel,
                                     int size_of_input_word,
                                     int bytes_in_data1,
                                     int offset_in_input_samples,
                                     int offset,
                                     int input_size,
                                     std::vector<std::vector<int> >& track_positions) {
  bool failure=false;
  int first_fail_position = -1;

  for (int i=0; i<n_channels && !failure; i++) {
    for (int j=0; j<output_bytes_per_channel && !failure; j++) {
      if (out1[i][j] != out2[i][j] && !failure) {
        first_fail_position = j;
        failure=true;
      }
    }
  }

  if (failure) {
    std::cout << std::endl;
    std::cout << "Failure detected :"<< std::endl;
    std::cout << "     subbands: " << track_positions.size() << std::endl;
    std::cout << "      fan out: " << track_positions[0].size() << std::endl;
    std::cout << "       offset: " << offset << std::endl;
    std::cout << "      size_of_input_word: " << size_of_input_word << std::endl;
    std::cout << " output_byte_per_channel: " << output_bytes_per_channel << std::endl;
    std::cout << "     index of the middle: " << bytes_in_data1 << std::endl;
    std::cout << " offset in input samples: " << offset_in_input_samples << std::endl;

    std::cout << "track_positions: " << std::endl;
    for (unsigned int i=0;i<track_positions.size();i++) {
      std::cout << "Channel "<<i<<": ";
      for (unsigned int j=0;j<track_positions[i].size();j++) {
        std::cout << track_positions[i][j] << ", ";
      }
      std::cout << std::endl;
    }



    std::cout << "First mismatch at: " << first_fail_position << std::endl;

    for (int i=0; i< input_size;i++) {
      if ( ((i+1) % size_of_input_word) ) {
        std::cout << " a. :";
      }
      print_hex(std::cout, in1[i]);
      std::cout << "(" << (int)in1[i] << ")";

      if ( !((i+1) % size_of_input_word) ) {
        std::cout <<"     :"<<i/size_of_input_word;
        std::cout << std::endl;
      } else {
        std::cout << " ";
      }
    }
    for (int i=0; i<input_size ;i++) {
      if ( ((i+1) % size_of_input_word) ) {
        std::cout << " b. :";
      }
      print_hex(std::cout, in2[i]);
      std::cout << "(" << (int)in2[i] << ")";

      if ( !((i+1) % size_of_input_word) ) {
        std::cout <<"     :"<<i/size_of_input_word;
        std::cout << std::endl;
      } else {
        std::cout << " ";
      }
    }
    std::cout << "-----------" << std::endl;


    for (int j=0; j<output_bytes_per_channel; j++) {
      for (int i=0; i<n_channels; i++) {
        if ( out1[i][j] != out2[i][j] )
          std::cout << STRGREEN;

        print_hex(std::cout, out1[i][j]);
        std::cout << " ";
        print_hex(std::cout, out2[i][j]);

        if ( out1[i][j] != out2[i][j] )
          std::cout << STRBLACK;
        std::cout << " | ";
      }
      std::cout << " address: " << j << std::endl;
    }
  }
  return !failure;
}


Channel_extractor_interface& Benchmark::get_extractor() {
  return channel_extractor;
}
