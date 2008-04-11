/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)

  void initialise(const std::vector< std::vector<int> > &track_positions_,
                  int size_of_one_input_word_,
                  int input_sample_size_);

  void extract(unsigned char *in_data1,
               unsigned char **output_data);
private:
  Channel_extractor_interface* hidden_implementation_;

  std::vector< std::vector<int> > track_positions;
  int size_of_one_input_word;
  int input_sample_size;
  int n_subbands;

  // Computed
  int fan_out;
};

#endif // CHANNEL_EXTRACTOR_5_H__