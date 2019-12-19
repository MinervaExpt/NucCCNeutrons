# NucCCNeutrons

## Description
Produces a cross section in neutron multiplicity from MINERvA data.  This is just the macro component that consumes AnaTuples from a Gaudi AnaTool of the same name.  Andrew Olivier's thesis project.  This version is based on Ben's New Systematics Framework.

My goal is to build on the machinery used to produce [MINERvA's low energy neutron paper](https://arxiv.org/abs/1901.04892) and [Miranda Elkins' masters thesis](http://inspirehep.net/record/1609603?ln=en) to:
1. Study neutrons from neutrino interactions as a function of nucleus using [MINERvA's](https://arxiv.org/abs/1305.5199) nuclear targets
2. Extract a differential cross section in neutron multiplicity as a function of nucleus

## Installation
1. Make a directory structure for the repository to live in.  I recommend an out of source build:
  * src
  * debug
  * opt
2. cd src
3. git clone https://github.com/MinervaExpt/NucCCNeutrons.git
4. cd ../debug && mkdir build && cd build && cmake ../../src -DCMAKE_INSTALL_PREFIX=`pwd`/.. #You might need to specify paths to dependencies too
5. make install

## Dependencies
1. [PlotUtils](https://cdcvs.fnal.gov/redmine/projects/minerva-sw/repository/show/AnalysisFramework/Ana/PlotUtils)
2. [ROOT](https://root.cern.ch/building-root)
3. [YAML-cpp](https://github.com/jbeder/yaml-cpp)
4. [BaseUnits](https://github.com/aolivier23/BaseUnits)
