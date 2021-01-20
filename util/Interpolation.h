//File: Interpolation.h
//Brief: A linear interpolation between key-value pairs from a file.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef INTERPOLATION_H
#define INTERPOLATION_H

//c++ includes
#include <map>
#include <fstream>

namespace util
{
  class Interpolation
  {
    public:
      Interpolation(std::ifstream& file);
  
      double operator [](const double key) const;
  
      bool empty() const;
  
    private:
      std::map<double, double> fTable;
  };
}

#endif //INTERPOLATION_H
