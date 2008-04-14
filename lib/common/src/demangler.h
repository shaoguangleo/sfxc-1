/* Copyright (c) 2007 Joint Institute for VLBI in Europe (Netherlands)
 * Copyright (c) 2007 University of Amsterdam (Netherlands)
 * All rights reserved.
 *
 * Author(s): Nico Kruithof <Kruithof@JIVE.nl>, 2007
 *            Damien Marchal <dmarchal@science.uva.nl>, 2007
 *
 *
 * This file is part of:
 *   - common library
 *
 * This file contains:
 *   - A demangler to manipulate human readable string
 *     instead of the mangled one generated by the compiler
 *     this code is higly non-portable.
 */
#ifndef DEMANGLER_H
#define DEMANGLER_H

#include <iostream>


/*********************************************
* @class Demangler
* @desc Convert compiler-specific naming to
* human readable format.
*
* Warning:
*    - it is not specified in GNU doc how
*      to know the size of the mangled string
*      large/fixed buffer size is allocated
**********************************************/
class Demangler {
public:
  // Create an demangler that will convert and hold
  // the sepecified mangled name given in parameter
  Demangler(const char* name);
  Demangler(const Demangler& param) {
    std::cout << "toto" <<std::endl;
  };
  Demangler(Demangler& param) {
    std::cout << "toto" <<std::endl;
  };

  // Release memory and destroy the object.
  ~Demangler();

  // return a reference to the unmangled name, the reference
  // is valid as long as the object exist.
  std::string& value();

  // put the unmangled name to a stream
  friend std::ostream& operator<<(std::ostream& str, Demangler& dem );
private:
  // string to store the mangled name
  std::string name_;

  // temporary buffer to store the unmangled name
  char*  buffer_;
  size_t size_;

};

// Function to get the typename of an object.
// usage:
//     MyClass object;
//     ...
//     GetTypeName(object); will return "MyClass"
template<class T>
std::string& get_type_name(T& data) {
  static Demangler g_dem( typeid(T).name() );
  return g_dem.value();
}


// Function to get the typename of an object.
// Use the template instead of parameter
// usage:
//     MyClass object;
//     ...
//     GetTypeName(object); will return "MyClass"
template<class T>
std::string& get_type_name() {
  static Demangler g_dem( typeid(T).name() );
  return g_dem.value();
}


#endif // DEMANGLER_H
