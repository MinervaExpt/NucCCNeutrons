//File: makeGeneralizedBirksTable.cpp
//Brief: One generalization of Birks' Law introduces a term in the denominator that is
//       quadratic in dE/dx.  MINERvA's simulation only uses the linear denominator form
//       of Birks' Law that PDG quotes.  This program takes a table of dE/dx values versus
//       particle energies and produces a table of ratios of the linear Birks' Law to the
//       quadratic version.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//Local includes
#include "util/Interpolation.h"

//c++ includes
#include <iostream>

#define USAGE "USAGE: makeGeneralizedBirksTable <dEdxTable.txt> <start> <end> <nSteps> [Cvalue] [kBValue]\n\n"\
              "dEdxTable.txt shall be a plaintext file with 2 space-separated columns:\n"\
              "<energy> <dEdx>\n"\
              "All values shall be convertible to floating point numbers.\n"\

int main(const int argc, const char** argv)
{
  if(argc < 5 || argc > 7)
  {
    std::cerr << "Wrong number of arguments.  makeGeneralizedBirksTable expects 4-6, but you passed " << argc-1 << ".\n\n"
              << USAGE << "\n";
    return 1;
  }

  std::ifstream dEdxFile(argv[1]);
  if(!dEdxFile)
  {
    std::cerr << "Could not open dEdx table at " << argv[1] << ".\n"
              << USAGE << "\n";
    return 2;
  }

  util::Interpolation dEdxTable(dEdxFile);
  if(dEdxTable.empty())
  {
    std::cerr << argv[1] << " is an empty dEdx table.\n" << USAGE << "\n";
    return 3;
  }

  try
  {
    const double start = std::stod(argv[2]),
                 end = std::stod(argv[3]);
    const int nSteps = std::stod(argv[4]);
  
    double C = 1e-5; //Default loosely based on range from https://www.sciencedirect.com/science/article/pii/0029554X70907688
    if(argc > 5)
    {
      C = std::stod(argv[5]);
    }
    double kB = 0.103; //From MINERvA testbeam in mm/MeV
    if(argc > 6)
    {
      kB = std::stod(argv[6]);
    }
    kB /= 10.; //mm/MeV -> cm/MeV to match the dE/dx units from ASTAR and PSTAR

    const double density = 1.05; //polystyrene density in g/cm^3
    const double stepSize = (end - start)/(double)nSteps;
  
    std::ofstream birksTable(std::string("birksRatiosFrom_") + argv[1]);
  
    auto birksLaw = [kB](const double dEdx)
                    { return 1./(1. + kB*dEdx); };
    auto generalBirks = [kB, C](const double dEdx)
                        {return 1./(1. + kB*dEdx + C*dEdx*dEdx); };
  
    //Estimate integral using the midpoint rule.
    double runningBirks = 0,
           runningGeneral = 0;
    for(int whichStep = 0; whichStep < nSteps; ++whichStep)
    {
      const double energy = start + stepSize*(whichStep + 0.5);
      const double dEdx = dEdxTable[energy]*density;
      runningBirks += stepSize * birksLaw(dEdx);
      runningGeneral += stepSize * generalBirks(dEdx);
      birksTable << energy << " " << runningGeneral / runningBirks;
      //birksTable << " " << runningBirks/energy << " " << runningGeneral/energy; //For debugging, print quenching factors too and compare to literature like https://arxiv.org/pdf/1111.2248.pdf
      birksTable << "\n";
    }
  }
  catch(const std::exception& e)
  {
    std::cerr << "Failed to convert a command line parameter to a number because:\n" << e.what() << "\n"
              << USAGE << "\n";
    return 4;
  }

  return 0;
}
