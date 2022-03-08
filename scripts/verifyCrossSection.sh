#!/bin/bash
#USAGE: verifyCrossSection.sh </path/to/tuples.root> [tagToCompare]

TUPLE_PATH=${1}
PATH_TO_SOURCE=${PLOTUTILSROOT}/../.. #Assumes that NucCCNeutrons is already set up
NUCCCNEUTRONS_TAG=${2:-$(git -C ${PATH_TO_SOURCE}/NucCCNeutrons describe --tags --abbrev=0)}
#TODO: tag MAT and MAT-MINERvA more often and use their tags too

mkdir tmp && cd tmp

#Set up reference development area
mkdir referenceSource && cd referenceSource
git clone https://github.com/MinervaExpt/MAT-MINERvA.git
git clone https://github.com/MinervaExpt/NucCCNeutrons.git --branch ${NUCCCNEUTRONS_TAG}
git clone https://github.com/MinervaExpt/GENIEXSecExtract.git

mkdir opt && cd opt
mkdir build && cd build
cmake ../../MAT-MINERvA/bootstrap -DCMAKE_INSTALL_PREFIX=`pwd`/.. -DCMAKE_BUILD_TYPE=Release
make install -j $(nproc)
cd ..

#Not needed for the basic test, but I might want it later
#mkdir buildGENIEXSecExtract && cd buildGENIEXSecExtract
#cmake ../../GENIEXSecExtract -DCMAKE_INSTALL_PREFIX=`pwd`/.. -DCMAKE_BUILD_TYPE=Release
#make install -j $(nproc)
#cd ..

mkdir buildNucCCNeutrons && cd buildNucCCNeutrons
cmake ../../NucCCNeutrons -DCMAKE_INSTALL_PREFIX=`pwd`/.. -DCMAKE_BUILD_TYPE=Release
make install -j $(nproc)
cd ..

cd ../..

#Run tuples using the reference installation
mkdir reference && cd reference
source referenceSource/opt/bin/setup.sh
ProcessAnaTuples multiNeutron.yaml ${TUPLE_PATH} &> reference.txt &
cd ..

#Run tuples using the current installation
mkdir current && cd current
source ${PATH_TO_SOURCE}/opt/bin/setup.sh #TODO: Does this override previous location of ProcessAnaTuples?
ProcessAnaTuples multiNeutron.yaml ${TUPLE_PATH} &> current.txt &
cd ..

wait #Wait for both processes to complete
#TODO: Save process numbers and use wait to check their exit codes

#Compare and emit exit code
python ${PATH_TO_SOURCE}/compareHistsExact.py reference/multiNeutronMC.root current/multiNeutronMC.root
