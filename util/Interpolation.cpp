//File: Interpolation.cpp
//Brief: A linear interpolation between key-value pairs from a file.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "util/Interpolation.h"

namespace util
{
  Interpolation::Interpolation(std::ifstream& file)
  {
    std::pair<double, double> nextEntry{-1, -1};
    while(file >> nextEntry.first)
    {
      file >> nextEntry.second;
      fTable.insert(nextEntry);
    }
  }

  //An Interpolation that always returns 1
  Interpolation::Interpolation()
  {
    fTable.insert(std::make_pair(0, 1));
    fTable.insert(std::make_pair(1, 1));
  }
  
  double Interpolation::operator [](const double key) const
  {
    const auto upper = fTable.lower_bound(key);
    const auto lower = std::prev(upper);
  
    return (upper->second - lower->second)/(upper->first - lower->first)*(key - lower->first) + lower->second;
  }
  
  bool Interpolation::empty() const
  {
    return fTable.empty();
  }
}
