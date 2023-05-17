#!/bin/bash

targetFileName=${1}
fileNameWithout2p2h=NeutronDetection_lowq3_target3_warps_2p2hOffMC.root #$(basename ${targetFileName} "MC.root")_no2p2hMC.root
dataFileNameWithout2p2h=$(basename ${fileNameWithout2p2h} "MC.root")Data.root
extDir="extended2p2h"

mkdir -p super
cd super
MergeAndScaleByPOT ../me6[G-J]/${fileNameWithout2p2h}
MergeAndScaleByPOT ../me6[G-J]/${dataFileNameWithout2p2h}
mv $(basename ${fileNameWithout2p2h} ".root")_merged.root ${fileNameWithout2p2h} #Merging script doesn't work well when the MC file has _merged in its name.  It looks for a Data file without _merged.
mv $(basename ${dataFileNameWithout2p2h} ".root")_merged.root ${dataFileNameWithout2p2h}

mkdir -p ${extDir} 
cd ${extDir}
cp ../../me6G/${extDir}/${targetFileName} .

cd ../..
mkdir -p merged
cd merged
echo "Moved to $(pwd) for final merging"
MergeAndScaleByPOT ../me5A/${fileNameWithout2p2h} ../me6[A-F]/${fileNameWithout2p2h} ../super/${fileNameWithout2p2h} ../me5A/${extDir}/${targetFileName} ../me6[A-F]/${extDir}/${targetFileName} ../super/${extDir}/${targetFileName}
