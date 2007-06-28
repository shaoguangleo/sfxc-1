/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * All rights reserved.
 * 
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 * 
 * $Id$
 *
 */

#include <Data_writer_tcp.h>
#include <assert.h>

#include <TCP_Connection.h>
#include <iostream>

// defines send:
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

Data_writer_tcp::Data_writer_tcp(int _port) 
: connection_socket(-1), socket(-1), port(_port)
{
  TCP_Connection connection;
  
  connection_socket = connection.open_port(port);
  while (connection_socket <= 0) {
    port ++;
    connection_socket = connection.open_port(port);
  }
}

void Data_writer_tcp::open_connection() {
  TCP_Connection connection;
  socket = connection.open_connection(connection_socket);
  assert(socket > 0);
}

Data_writer_tcp::~Data_writer_tcp() {
  close(socket);
}
  
size_t
Data_writer_tcp::do_put_bytes(size_t nBytes, char *buff) {
  if (socket <= 0) return 0;
  assert(nBytes > 0);
  ssize_t result = write(socket, buff, nBytes);
  if (result <= 0) return 0;
  assert((size_t)result == nBytes);
  return result;
}


unsigned int Data_writer_tcp::get_port() {
  return port;
}  

