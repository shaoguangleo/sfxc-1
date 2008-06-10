#!/usr/bin/python

# Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
# All rights reserved.
#  
# Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
# 
# $Id$

import sys, os,time, filecmp;

# Load the ccf files for testing
RC_FILE = os.path.join(os.environ.get('HOME'), ".sfxcrc")
if os.path.isfile(RC_FILE):
  execfile(RC_FILE)

if (os.environ.get("MAKE")):
  status = os.system('$MAKE test_input_node')
else:
  status = os.system("make test_input_node")
if (status != 0): 
  sys.exit(1)

for [ctrl_file,vex_file] in controlfiles:
  cmd = "mpirun -np 3 ./test_input_node " + \
                     ctrl_file+" "+vex_file+" "+tmp_output_directory
  print cmd
  status = os.system(cmd)
  if (status != 0):
    print "test_input_node: returned error."
    sys.exit(1);

sys.exit(0);