/* author : N.G.H. Kruithof
*/

#include <iostream>
#include <complex>
#include <stdio.h>
#include <fstream>
#include <assert.h>
#include <fftw3.h>
#include <math.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <utils.h>
#include <Log_writer_cout.h>
#include <gnuplot_i.h>

#include <runPrms.h>
#include <genPrms.h>
#include <staPrms.h>
#include <constPrms.h>

RunP RunPrms;
GenP GenPrms;
StaP StaPrms[NstationsMax];

struct Plot_data {
  std::string job_name;
  double frequency;
  char sideband;
  std::vector<std::string> autos, crosses;
};

void plot(char *filename, int nPts, double *values, char *title) {
  char cmd[80];
  gnuplot_ctrl * g = gnuplot_init();

  gnuplot_cmd(g, "set terminal png medium size 300,200");
  snprintf(cmd, 80, "set output \"%s\"", filename);
  gnuplot_cmd(g, cmd);
  gnuplot_setstyle(g, "lines");
  gnuplot_plot_x(g, values, nPts, title) ;
  gnuplot_close(g);
}

void produce_plots(char *ctrl_file, Plot_data &data) {
  int err, plotNr = 0;

  Log_writer_cout log_writer;

  err = initialise_control(ctrl_file, log_writer, RunPrms, GenPrms, StaPrms);
  if (err != 0) return;

  log_writer(0) << GenPrms.get_corfile() << std::endl;
  
  data.job_name = GenPrms.get_job();
  data.frequency = GenPrms.get_skyfreq();
  data.sideband = (GenPrms.get_sideband()-1 ? 'L' : 'U');
  std::ifstream infile(GenPrms.get_corfile(), ios::in | ios::binary);
  assert(infile.is_open());

  int nLags = GenPrms.get_n2fft()+1;
  std::complex<double> in[nLags], out[nLags];
  double magnitude[nLags];
  
  char polarisation = 'l';

  std::vector<std::string> station_names;
  // Auto correlations 
  for (int i=0; i<GenPrms.get_nstations(); i++) {
    for  (int j=0; j<nLags; j++) {
      infile.read((char *)&in[j], 2*sizeof(double));
      magnitude[j] = abs(in[j]);
    }
    char title[80], filename[80];
    if ((i!=0) && 
        (!strcmp(StaPrms[0].get_stname(), StaPrms[i].get_stname()))) {
      polarisation = 'r';
    }
    char station_name[10];
    snprintf(station_name, 10, "%s-%c", StaPrms[i].get_stname(), polarisation);
    station_names.push_back(std::string(station_name));
    
    snprintf(title, 80, "Auto %s", station_names[i].c_str());
    snprintf(filename, 80, "%d_%s_%s.png", 
             plotNr, GenPrms.get_job(), station_names[i].c_str());
    data.autos.push_back(filename);
    
    plotNr ++;
    plot(filename, nLags, magnitude, title);    
  }
  
  fftw_plan visibilities2lags; 
  visibilities2lags = fftw_plan_dft_1d(nLags, 
                                       reinterpret_cast<fftw_complex*>(&in),
                                       reinterpret_cast<fftw_complex*>(&out),
                                       FFTW_BACKWARD, 
                                       FFTW_ESTIMATE);
  
  // Cross correlations
  if (RunPrms.get_ref_station() == -1) {
    for (int i=0; i<GenPrms.get_nstations(); i++) {
      for (int j=i+1; j<GenPrms.get_nstations(); j++) {
        for (int k=0; k<nLags; k++) {
          infile.read((char *)&in[k], 2*sizeof(double));
        }
        fftw_execute(visibilities2lags);
        for  (int k=0; k<nLags; k++) {
          magnitude[k] = abs(out[(k+nLags/2)%nLags]);
        }
        
        char title[80], filename[80];
        snprintf(title, 80, "Cross %s vs. %s", 
                 station_names[i].c_str(), 
                 station_names[j].c_str());
        snprintf(filename, 80, "%3d_%s_%s-%s.png", 
                 plotNr,
                 GenPrms.get_job(), 
                 station_names[i].c_str(), 
                 station_names[j].c_str());
        data.crosses.push_back(filename);
        plotNr++;

        plot(filename, nLags, magnitude, title);
      }    
    }
  } else {
    int ref_station = RunPrms.get_ref_station();
    for (int i=0; i<GenPrms.get_nstations(); i++) {
      if (i != ref_station) {
        for (int k=0; k<nLags; k++) {
          infile.read((char *)&in[k], 2*sizeof(double));
        }
        fftw_execute(visibilities2lags);
        for  (int k=0; k<nLags; k++) {
          magnitude[k] = abs(out[(k+nLags/2)%nLags]);
        }
        
        char title[80], filename[80];
        snprintf(title, 80, "Cross %s vs. %s", 
                 station_names[ref_station].c_str(), 
                 station_names[i].c_str());
        snprintf(filename, 80, "%3d_%s_%s-%s.png", 
                 plotNr,
                 GenPrms.get_job(), 
                 station_names[ref_station].c_str(), 
                 station_names[i].c_str());
        data.crosses.push_back(filename);
        plotNr++;

        plot(filename, nLags, magnitude, title);
      }
    }    
  }
  
  fftw_destroy_plan(visibilities2lags);
}

void print_html(std::ostream &html_output, Plot_data &data) {
  html_output << "<h1>" << data.job_name << "</h1>" << std::endl;
  html_output << "<pre>Frequency: " << data.frequency << "<br>"
              << "Sideband: " << data.sideband << "</pre>"
              << std::endl;

  html_output << "<h3>Auto correlations</h3>" << std::endl;
  for (size_t i=0; i<data.autos.size(); i++) {
    html_output << "<img src='" << data.autos[i] << "'>" << std::endl;
  }

  html_output << "<h3>Cross correlations</h3>" << std::endl;
  for (size_t i=0; i<data.crosses.size(); i++) {
    html_output << "<img src='" << data.crosses[i] << "'>" << std::endl;
  }
  
  html_output << "<HR>" << std::endl;
}

//main
int main(int argc, char *argv[])
{
  std::ofstream html_output("index.html");
  html_output << "<html><head><title>SFXC output</title></head><body>" << std::endl;
  
  for (int i=1; i<argc; i++) {
    Plot_data data;
    produce_plots(argv[i], data);

    print_html(html_output, data);
  }

  html_output << "</body></html>" << std::endl;

  return 0;
}