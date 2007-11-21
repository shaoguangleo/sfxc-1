#include "delay_correction.h"

Delay_correction::Delay_correction()
  : output_buffer(Output_buffer_ptr(new Output_buffer(10))),
    current_time(-1),
    delay_table_set(false),
    verbose(false)
{
}

Delay_correction::~Delay_correction() {
  DEBUG_MSG(delay_timer);
}

void Delay_correction::connect_to(Input_buffer_ptr new_input_buffer) {
  assert(input_buffer == Input_buffer_ptr());
  input_buffer = new_input_buffer;
}

void Delay_correction::set_delay_table(const Delay_table_akima &delay_table_) {
  delay_table_set = true;
  delay_table = delay_table_;
}

void Delay_correction::do_task() {
  delay_timer.resume();
  assert(delay_table_set);
  assert(current_time >= 0);

  int input_size;
  Input_buffer_element &input = input_buffer->consume(input_size);
  Output_buffer_element &output = output_buffer->produce();
  assert(input_size == number_channels());
  assert(input.size() == number_channels());

  // A factor of 2 for padding
  if (output.size() < input_size*2) {
    output.resize(input_size*2);
  }

  // Get the input data into a temporary buffer
  memmove(&intermediate_buffer[0], 
          &intermediate_buffer[number_channels()],
          (intermediate_buffer.size()-number_channels())*sizeof(double));
  for (int in = 0, out = intermediate_buffer.size()-number_channels();
       in < number_channels(); 
       in++, out++) {
//    assert(out < intermediate_buffer.size());
    intermediate_buffer[out] = input[in];
  }

  double delay = delay_table.delay(current_time+length_of_one_fft()/2);


  // Do the integer delay correction
  double delay_in_samples = delay*sample_rate();
  int integer_delay = (int)std::floor(delay_in_samples+.5);
  assert(integer_delay <= 0);
  assert(-(intermediate_buffer.size()-number_channels()) <= integer_delay);
  assert(frequency_buffer.size() == number_channels()*2);
  for (int in = intermediate_buffer.size()-number_channels()+integer_delay, out = 0;
       out < number_channels(); 
       in++, out++) {
    // Implicit double to complex conversion
    frequency_buffer[out] = intermediate_buffer[in];
  }
  

  fractional_bit_shift(&frequency_buffer[0], 
                       integer_delay,
                       delay_in_samples - integer_delay);

  fringe_stopping(&frequency_buffer[0],
                  output.buffer());

  current_time += length_of_one_fft();
  
  input_buffer->consumed();
  output_buffer->produced(2*number_channels());

  delay_timer.stop();
}


void Delay_correction::fractional_bit_shift(std::complex<double> output[],
                                            int integer_shift,
                                            double fractional_delay) {
  // create the fft-plans
  plan_t2f = fftw_plan_dft_1d(number_channels(), 
                              (fftw_complex *)output, (fftw_complex *)output, 
                              FFTW_BACKWARD, FFTW_ESTIMATE);
  plan_f2t = fftw_plan_dft_1d(number_channels(), 
                              (fftw_complex *)output, (fftw_complex *)output, 
                              FFTW_FORWARD,  FFTW_ESTIMATE);

  // 3) execute the complex to complex FFT, from Time to Frequency domain
  //    input: sls. output sls_freq
  fftw_execute(plan_t2f);

  output[0] *= 0.5;
  output[number_channels()/2] *= 0.5;//Nyquist

  // 4c) zero the unused subband (?)
  for (int i=number_channels()/2+1; i<number_channels(); i++){
    output[i] = 0.0;
  }

  // 5a)calculate the fract bit shift (=phase corrections in freq domain)
  double tmp1 = -2.0*M_PI*fractional_delay/sample_rate();
  double tmp2 = 0.5*M_PI*integer_shift;/* was: / ovrfl */
  // 5b)apply phase correction in frequency range
  //assert(freq_scale.size() == number_channels()+1);
  for (size_t i = 0; i < freq_scale.size(); i++){
    //phi  = -2.0*M_PI*dfs*tbs*fs[jf] + 0.5*M_PI*integer_shift/ovrfl;
    double phi  = tmp1*freq_scale[i] + tmp2;
    std::complex<double> tmp(cos(phi),sin(phi));
    output[i] *= tmp;
  }
  
  // 6a)execute the complex to complex FFT, from Frequency to Time domain
  //    input: sls_freq. output sls
  fftw_execute(plan_f2t);
}
void Delay_correction::fringe_stopping(std::complex<double> input[],
                                       double output[]) {
  double mult_factor_phi =
    -sideband()*2.0*M_PI*(channel_freq() + sideband()*bandwidth()*0.5);
  
  int64_t time = current_time;

  int64_t delta_time = ((((int64_t)n_recompute_delay)*1000000)/sample_rate());
  if (delta_time < 1) delta_time = 1;
  double phi, cosPhi=0, sinPhi=0, deltaCosPhi=0, deltaSinPhi=0;
  // Initialise the end values
  double phi_end = mult_factor_phi * delay_table.delay(time);
  double cosPhi_end = cos(phi_end);
  double sinPhi_end = sin(phi_end);

  for (int i=0; i<number_channels(); i++) {
    if ((i % n_recompute_delay) == 0) {
      phi = phi_end;
      cosPhi = cosPhi_end;
      sinPhi = sinPhi_end;

      phi_end = 
        mult_factor_phi * delay_table.delay(time+delta_time);

      if (std::abs(phi_end-phi) < 0.4*maximal_phase_change) {
        // Sampling is too dense
        n_recompute_delay *= 2;
        delta_time = (((int64_t)n_recompute_delay)*1000000)/sample_rate();

        phi_end = 
          mult_factor_phi * delay_table.delay(time+delta_time);
      }

      while ((std::abs(phi_end-phi) > maximal_phase_change) &&
             (n_recompute_delay >= 2*sample_rate()/1000000)) {
        // Sampling is not dense enough
        n_recompute_delay /= 2;
        delta_time = (((int64_t)n_recompute_delay)*1000000)/sample_rate();

        phi_end =  mult_factor_phi * delay_table.delay(time+delta_time);
      }
      
      time += delta_time;
      
      cosPhi_end = cos(phi_end);
      sinPhi_end = sin(phi_end);

      deltaCosPhi = (cosPhi_end-cosPhi)/n_recompute_delay;
      deltaSinPhi = (sinPhi_end-sinPhi)/n_recompute_delay;

    
    }
    
    // 6b)apply normalization and multiply by 2.0
    // NHGK: Why only the real part
    input[i].real() *= 2.0;
        
    // 7)subtract dopplers and put real part in Bufs for the current segment
    output[i] = 
      input[i].real()*cosPhi - input[i].imag()*sinPhi;
    cosPhi += deltaCosPhi;
    sinPhi += deltaSinPhi;

  }
}

bool Delay_correction::is_ready_for_do_task() {
  return (!input_buffer->empty()) && (!output_buffer->full());
}

Delay_correction::Output_buffer_ptr 
Delay_correction::get_output_buffer() {
  assert(output_buffer != Output_buffer_ptr());
  return output_buffer;
}

void 
Delay_correction::set_parameters(const Correlation_parameters &parameters) {
  correlation_parameters = parameters;

  current_time = parameters.start_time*(int64_t)1000;

  assert((((int64_t)number_channels())*1000000000)%sample_rate() == 0);
 
  // NHGK: TODO: Check if it is big enough, 20 milisecond
  intermediate_buffer.resize((int)(.02*sample_rate()),0);

  //double dfr  = 1.0/(n2fftDC*tbs); // delta frequency
  double dfr  = sample_rate()*1.0/number_channels(); // delta frequency
  freq_scale.resize(number_channels()/2+1);
  
  for (size_t i=0; i<freq_scale.size(); i++) {
    //frequency scale in the segment
    //fs[jf]=sideband*(jf*dfr-0.5*GenPrms.get_bwfl()-GenPrms.get_foffset());
    freq_scale[i] = sideband()*(i*dfr-0.5*bandwidth());
  }
  
  n_recompute_delay = sample_rate()/1000000;
  
  frequency_buffer.resize(number_channels()*2);
  
  plan_t2f_orig = fftw_plan_dft_1d(number_channels(), 
                                   (fftw_complex *)NULL, (fftw_complex *)NULL, 
                                   FFTW_BACKWARD, FFTW_ESTIMATE);
  plan_f2t_orig = fftw_plan_dft_1d(number_channels(), 
                                   (fftw_complex *)NULL, (fftw_complex *)NULL, 
                                   FFTW_FORWARD,  FFTW_ESTIMATE);
}

int Delay_correction::number_channels() {
  return correlation_parameters.number_channels;
}
int Delay_correction::bandwidth() {
  return correlation_parameters.bandwidth;
}
int Delay_correction::sample_rate() {
  return correlation_parameters.sample_rate;
}
int Delay_correction::length_of_one_fft() {
  return (((int64_t)number_channels())*1000000)/sample_rate();
}
int Delay_correction::sideband() {
  assert((correlation_parameters.sideband == 'L') ||
         (correlation_parameters.sideband == 'R'));
  return (correlation_parameters.sideband == 'L' ? -1 : 1);
}
int64_t Delay_correction::channel_freq() {
  return correlation_parameters.channel_freq;
}