#include <iostream>
#include <complex>
#include <stdio.h>
#include <fstream>
#include <assert.h>
#include <fftw3.h>
using namespace std;

#define N 257
#include <fstream>

int main(int argc, char *argv[])
{
  assert(argc == 2);

  // Example of a fourier transform
  complex<double> in[N], out[N];
  fftw_plan p, p2;
  
  std::ifstream infile(argv[1], std::ios::in | std::ios::binary);
  assert(infile.is_open());

  std::ofstream fout("out.txt");
  assert(fout.is_open());

  p = fftw_plan_dft_1d(N, 
                       reinterpret_cast<fftw_complex*>(&in),
                       reinterpret_cast<fftw_complex*>(&out),
                       FFTW_BACKWARD, 
                       FFTW_ESTIMATE);

  bool finished = false;
  while (!finished) {
    // read in one fourier segment
    for  (int i=0; i<N; i++) {
      infile.read((char *)&in[i], 2*sizeof(double));
    }
    // check whether we are finished
    finished = infile.eof();

    if (!finished) {
      fftw_execute(p); /* repeat as needed */
      
      for (int i=0; i<N; i++) {
        fout << in[i].real() << " \t" << in[i].imag() << " \t" 
             << out[(i+N/2)%N].real() << " \t" << out[(i+N/2)%N].imag() << " \t" 
             << std::endl;
      }
    }
  }

  fftw_destroy_plan(p);
  
  return 0;
}
