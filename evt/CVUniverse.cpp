//File: CVUniverse.cpp
//Brief: A CVUniverse is a systematic universe with no shifts.  So, it accesses
//       AnaTuple variables from the Central Value.  It is a DefaultCVUniverse
//       to serve as my entry point into the New Systematics Framework.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Event model includes
#include "event/CVUniverse.h"

namespace evt
{
  CVUniverse::CVUniverse(const std::string& blobAlg, PlotUtils::ChainWrapper* chw, const double nsigma = 0): DefaultCVUniverse(chw, nsigma), m_blobAlg(blobAlg)
  {
  }
}
