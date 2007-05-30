/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#ifndef TCP_CONNECTION_H_
#define TCP_CONNECTION_H_

#include "types.h"

#include <vector>
#include <string>

class TCP_Connection
{
public:
	TCP_Connection(bool verbose = false);
	virtual ~TCP_Connection();
  
  /// Open a port on the server size
  int open_port(unsigned short int port);
  /// Open a connection on the server size
  unsigned int open_connection(int socket);
  
  /// Client side connect
  int do_connect(UINT64, unsigned short int port);
  int do_connect(const char *hostname, unsigned short int port);
  
  void get_ip_addresses(std::vector<UINT64> &addr);
  void get_ip_addresses(std::vector<std::string> &addr);
  
  bool is_localhost(UINT64 ip_addr) {
    return (ip_addr == 0x100007f);
  }
  bool is_localhost(const char *hostname) {
    return (hostname == "127.0.0.1");
  }
  std::string ip_addr(UINT64 ip) {
    char addr[16];
    sprintf(addr, "%d.%d.%d.%d", 
            (int)ip&255, (int)(ip>>8)&255, (int)(ip>>16)&255, (int)ip>>24);
    return std::string(addr);
  }
private:
  bool verbose;
};

#endif /*TCP_CONNECTION_H_*/