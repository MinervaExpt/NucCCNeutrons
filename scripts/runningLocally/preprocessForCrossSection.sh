#!/bin/bash

originalCrossSectionFile=${1}
neutronBandName="NeutronInelasticExclusives"
crossTalkBandName="UnifiedCrossTalk"

originalCrossSectionBaseName=$(basename ${originalCrossSectionFile} ".root")

#First, make a backup of the original cross section file before I start removing things from it.
cp ${originalCrossSectionFile} ${originalCrossSectionBaseName}_afterMoNARemoved.root

#Turn neutron interaction systematics into a 2-universe band.  1-universe error bands don't work in the MAT.
SwapSysUnivWithCV ${originalCrossSectionBaseName}_afterMoNARemoved.root ${neutronBandName}
python removeMoNABand.py ${originalCrossSectionBaseName}_afterMoNARemoved.root
SpecialSampleAsErrorBand ${originalCrossSectionBaseName}_afterMoNARemoved.root ${originalCrossSectionBaseName}_afterMoNARemoved.root ${neutronBandName} ${originalCrossSectionBaseName}_afterMoNARemoved_${neutronBandName}_0.root

#Insert special cross-talk sample as a 2-universe error band
SpecialSampleAsErrorBand ${originalCrossSectionBaseName}_afterMoNARemoved_with_${neutronBandName}.root ../me6A/multiNeutron_MnvTunev1MC.root ${crossTalkBandName} ../crossTalkUpME6A/multiNeutron_MnvTunev1MC.root

#TODO: Take out 2p2h universe 2?

#Remove intermediate files
rm ${originalCrossSectionBaseName}_afterMoNARemoved_${neutronBandName}_0.root
rm ${originalCrossSectionBaseName}_afterMoNARemoved.root
rm ${originalCrossSectionBaseName}_afterMoNARemoved_with_${neutronBandName}.root
#rm ${originalCrossSectionBaseName}_afterMoNARemoved_with_${neutronBandName}_with_${crossTalkBandName}.root
