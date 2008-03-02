/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 * This file contains a little file client based on a simple
 * protocol. The application simply connect to a dnfp server
 * and request a file. 
 */
#include <cassert>
#include <iostream>
#include "thread.h"
#include "data_writer_socket.h"
#include "data_reader_socket.h"
#include "data_reader_blocking.h"
#include "data_reader_dnfp.h"
#include "connexion_listener_thread.h"
#include "monitor.h"
#include "exception_common.h"

class Client_reader  : public Thread {
public:
  Client_reader(Data_reader_dnfp* reader) : m_reader(reader) {
    m_buffersize = 10000;
    m_buffer = new char[m_buffersize];
    m_monitor.set_name("data-reader");
  }
  void do_execute() {
    int count=100000;
    while ( !m_reader->eof() && count-- > 0) {
      //std::cout << "Still reading bytes :" << m_buffersize << std::endl;
      if ( count % 1000 == 0) m_monitor.begin_measure();
      int val = m_reader->get_bytes(m_buffersize, m_buffer);
      assert(val == m_buffersize);
      if ( count % 1000 == 0) m_monitor.end_measure(val);
      if ( count % 1000 == 0) std::cout << "reading speed:" << m_monitor.last_measure() << std::endl;
    }
    m_reader->closef();

    std::cout << "EOF" << std::endl;
  }


private:
  Data_reader_dnfp* m_reader;

  char* m_buffer;
  unsigned int m_buffersize;

  QOS_MonitorSpeed m_monitor;
};

typedef Client_reader* Client_reader_ptr;


void print_usage(char* appname) {
  std::cerr << "Syntax: "<< appname << "  dnfp://<host>:<port>/<fileid>" << std::endl;
}

/*
  Client for the SC07 demo
*/
int main(int argc, char** argv) {
  try {
    InterfaceIP* interface = Network::get_any_interface();

    int port=2630;

    if ( argc != 2 ) {
      print_usage(argv[0]);
      exit(1);
    }

    String url=String(argv[1]);

    Data_reader_dnfp dnf(interface, url);
    Client_reader reader(&dnf);
    reader.start();

    ThreadPool::s_wait_for_all_termination();
  } catch (Exception& ex) {
    std::cout << ex << std::endl;
  }
}
