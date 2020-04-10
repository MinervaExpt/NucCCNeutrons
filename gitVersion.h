//File: gitVersion.h
//Brief: Interface to version control status from c++.  Filled in at build time.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <string>

#ifndef GIT_GITVERSION_H
#define GIT_GITVERSION_H

namespace git
{
  std::string commitHash();
}

#endif //GIT_GITVERSION_H
