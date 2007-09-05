/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Ruud Oerlemans <Oerlemans@JIVE.nl>, 2007
 * 
 * $Id$
 *
 * Class definitions for general parameters
 */

#ifndef GEN_PRMS_H
#define GEN_PRMS_H

#include <types.h>
#include <string>
#include "Log_writer.h"

class MPI_Transfer;

class GenP
{
  friend class MPI_Transfer;
  public:

    //default constructor, set default values for Mk4 control parameters
    GenP();

    //parse control file c
    int parse_ctrlFile(const char *ctrlFile, Log_writer&log_writer);

    //check control parameters
    int check_params(Log_writer &log_writer) const;

    //get functions
    
    /** Name of the experiment. **/
    char* get_experiment() const;
    
    /** Name of the job. **/
    char* get_job() const;
    
    /** Substitute random values for the header **/
    int   get_rndhdr() const;

    /** year in the starttime. **/
    int   get_yst() const;
    /** day in the starttime. **/
    int   get_dst() const;
    /** hour in the starttime. **/
    int   get_hst() const;
    /** minutes in the starttime. **/
    int   get_mst() const;
    /** seconds in the starttime. **/
    int   get_sst() const;
    /** miliseconds in the starttime. **/
    int   get_milisst() const;

    /** duration of the experiment / correlated data (sec) **/
    int   get_duration() const;
    /** duration of the experiment / correlated data (usec) **/
    int64_t get_usDur() const;
    
    /** The number of stations **/
    int   get_nstations() const;
    /** Directory to put the correlated files **/
    char* get_outdir() const;
    /** Name of the logfile. **/
    char* get_logfile() const;
    /** Name of the correlation output file. **/
    const char* get_corfile() const;
    
    /** Sky frequency in Hz. **/
    double get_skyfreq() const;
    /** Input band width in Hz. **/
    double get_bwin() const;
    /** Length of a Fourier segment in delay correction. **/
    int   get_lsegm() const;
    /** Frequency offset. **/
    double get_foffset() const;
    /** Enable the CDE column in the delay table. **/
    int   get_cde() const;
    /** Enable the MDE column in the delay table. **/
    int   get_mde() const;
    /** Enable the RDE column in the delay table. **/
    int   get_rde() const;

    /** Filter data (settings the same for all stations)
        01-09-2006: filter not implemented in sfxc. **/
    int   get_filter() const;
    /** Bandwidth after optional filtering, Default BWFL=BWIN if FILTER=0. **/
    double get_bwfl() const;
    /** Filter start frequency, Default STARTF=0 if FITLER=0. **/
    double   get_startf() const;
    /** Filter resolution in Hz. **/
    double   get_deltaf() const;
    /** Filter oversampling. Default OVRFL=1 if FILTER=0 **/
    int   get_ovrfl() const;

    /** Length of Fourier segment in correlation. **/
    int   get_n2fft() const;
    /** overlap parameter for Fourier segments in correlation.
     * 0: no overlap. 
     **/
    float get_ovrlp() const;
    /** Time (sec) to average in correlation**/
    float get_time2avg() const;
    /** Time (usec) to average in correlation**/
    int64_t get_usTime2Avg() const;
    /** Padding with zeros in correlation.
     * 1: lenght = n2fft, no padding
     * 2: length = 2 * n2fft, padding with n2fft zeros
     **/
    int   get_pad() const;

    int get_sideband() const;

    /** Get the start time in microseconds
        from the beginning of the day 00:00. **/
    int64_t get_usStart() const;

    //set functions
    
    /** Set the earliest possible time for correlation 
    * in microseconds from the beginning of the day. **/
    void  set_usStart(int64_t);
    void  set_duration(int);

    /** Set the name of the correlation output file. **/
    void set_corfile(char *filename);

    
  private:
  
    //member functions
    
    // Set the start time in microseconds from the beginning of the day.
    void set_start(std::string Time);

    //data members

    int  rndhdr;   //random header on if 1 and off if 0
    
    char  *experiment; //name of the experiment
    char  *job;        //name of the job
    
    int   yst;        //start year
    int   dst;        //start day
    int   hst;        //start hour
    int   mst;        //start minute
    int   sst;        //start second
    int   milisst;    //start milisecond
    
    int   duration;   //duration of the experiment/correlated data (sec)
    int64_t usDur;      //duration of the experiment/correlated data (usec)
    
    int   nstations;  //number of radio telescope stations
    char  *outdir;    //output data directory
    char  *logfile;   //log file name
    std::string corfile;   //correlator product file

    double skyfreq;    //skyfrequency in Hertz
    double bwin;       //band width input in Hertz
    int    lsegm;      //length of a Fourier segment in a delay correction
    double foffset;    //frequency offset in Hertz
    int    cde;        //enable CDE column in delay table
    int    mde;        //enable MDE column in delay table
    int    rde;        //enable RDE column in delay table
    
    int    filter;     //enable filtering
    double bwfl;       //band width filter in Hertz
    double startf;     //filter start frequency in Hertz
    double deltaf;     //filter resolution in Hertz
    int    ovrfl;      //enable oversampling

    int    n2fft;      //length of Fourier segment in correlation
    float  ovrlp;      //overlap parameter for Fourier segmetns in correlation
    float  time2avg;   //time to average the correlation in sec
    int64_t  usTime2Avg; //time to average the correlation in usec
    int    pad;        //padding with zeros in correlation

    int64_t  usStart;    //start time in micro seconds without year and day

    int32_t sideband;

};


#endif // GEN_PRMS_H