//File: StreamRedirection.h
//Brief: A std::lock_guard<>-like object that sends an iostream to a file.
//       Useful for decluttering STDOUT by sending output from PlotUtils to
//       a file in a specific scope.
//
//       Destroy (i.e. let it go out of a {}-denoted scope) a StreamRedirection
//       to send the stream back to where it was combing from before.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#ifndef UTIL_STREAMREDIRECTION_H
#define UTIL_STREAMREDIRECTION_H

//c++ includes
#include <ostream>
#include <string>
#include <fstream>

namespace util
{
  class StreamRedirection
  {
    public:
      StreamRedirection(std::ostream& toRedirect, const std::string& fileName);
      ~StreamRedirection();      

    private:
      //N.B.: fRedirected = toRedirect had better outlive this object.
      //      The alternative is descent into undefined behavior.
      std::ostream& fRedirected; //A copy of toRedirect
      std::ofstream fRedirectTarget; //Where toRedirect is redirected to
      std::streambuf* fOldStreambuf; //To be restored in destructor
  };
}

#endif //UTIL_STREAMREDIRECTION_H
