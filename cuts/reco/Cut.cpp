//File: Cut.cpp
//Brief: A Cut decides whether a CVUniverse should be considered
//       reco signal.  Cuts can be used to define
//       Signals, Sidebands, and backgrounds.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//cuts includes
#include "cuts/reco/Cut.h"

//event includes
#include "evt/CVUniverse.h"

namespace reco
{
  bool Cut::operator ()(const evt::CVUniverse& event)
  {
    ++fEventsEntered;
    const bool result = passesCut(event);
    fEventsPassed += result;

    return result;
  }

  void Cut::enlargeTable(TableConfig& config) const
  {
    config.largestNameSize = std::max(config.largestNameSize, fName.length());
    config.largestPassedSize = std::max(config.largestPassedSize, std::log10(fEventsPassed));
  }

  void Cut::printTableRow(std::ostream& os, const TableConfig config) const
  {
    os << std::setw(config.largestNameSize) << fName << " | "
       << std::setw(config.largestPassedSize) << fEventsPassed << " | "
       << std::setprecision(config.nDecimals) << fEventsPassed / fEventsEntered << "\n";
  }

  void Cut::printTableHeader(std::ostream& os, TableConfig& config)
  {
    //Enlarge config to fit table headings if necessary
    const std::string nameHeading = "Name", passedHeading = "Events Passed",
                      percentHeading = "Percent Passed";
    config.largestNameSize = std::max(config.largestNameSize, nameHeading.length());
    config.largestPassedSize = std::max(config.largestPassedSize, passedHeading.length());
    config.nDecimals = std::max(config.nDecimals, config.percentHeading.length());

    os << std::setw(config.largestNameSize) << nameHeading << " | "
       << std::setw(config.largestPassedSize) << passedHeading << " | "
       << std::setw(config.nDecimals) << percentHeading << "\n";
  }
}
