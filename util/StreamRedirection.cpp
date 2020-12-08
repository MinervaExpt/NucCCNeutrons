//File: StreamRedirection.cpp
//Brief: A std::lock_guard<>-like object that sends an iostream to a file.
//       Useful for decluttering STDOUT by sending output from PlotUtils to
//       a file in a specific scope.
//
//       Destroy (i.e. let it go out of a {}-denoted scope) a StreamRedirection
//       to send the stream back to where it was combing from before.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "util/StreamRedirection.h"

namespace util
{
  StreamRedirection::StreamRedirection(std::ostream& toRedirect, const std::string& fileName): fRedirected(toRedirect), fRedirectTarget(fileName), fOldStreambuf(toRedirect.rdbuf())
  {
    toRedirect.rdbuf(fRedirectTarget.rdbuf());
  }

  StreamRedirection::~StreamRedirection()
  {
    fRedirected.rdbuf(fOldStreambuf);
  }
}
