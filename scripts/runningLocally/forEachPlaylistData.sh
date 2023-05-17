#!/usr/bin/bash

tupleDir="/media/anaTuples/withODRecoilFix"
for subDir in ${tupleDir}/*
do
  playlist=$(basename ${subDir})
  echo "Submitting for playlist ${playlist}: ProcessAnaTuples $1 ${tupleDir}/${playlist}/data/*.root &> ${1}.txt &"
  mkdir -p ${playlist}
  cd ${playlist}
  cp ../$1 .
  ProcessAnaTuples $1 ${tupleDir}/${playlist}/data/*.root &> ${1}.txt &
  cd ..
done
