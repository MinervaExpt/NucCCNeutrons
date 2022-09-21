#!/bin/bash

tupleDir="/media/anaTuples/withODRecoilFix"
for subDir in ${tupleDir}/*
do
  playlist=$(basename ${subDir})
  echo "Submitting for playlist ${playlist}: ProcessAnaTuples $1 ${tupleDir}/${playlist}/extended2p2h/*.root &> ${1}.txt &"
  mkdir -p ${playlist}/extended2p2h
  cd ${playlist}/extended2p2h
  cp ../../$1 .

  #Wait for the load average to be reasonable before submitting the next job
  #while [ $(cat /proc/loadavg | cut --fields 1 -s --delimiter=.) -gt $(nproc) ]
  while [ $(ps -u $USER | grep ProcessAna | wc -l) -ge $(nproc) ]
  do
    sleep 30
  done
  ProcessAnaTuples $1 ${tupleDir}/${playlist}/extended2p2h/*.root &> ${1}.txt &
  ln -s multiNeutron_noSystsData.root $(basename $1 ".yaml")Data.root #Makes POT scaling easier for warping studies
  cd ../..
done
