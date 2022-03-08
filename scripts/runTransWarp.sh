#!/bin/bash
MIGRATION_FILE=multiNeutronMC_merged.root
TRUE_HIST=Tracker_MuonPTSignal_EfficiencyNumerator
WARPED_FILE=$1
RECO_HIST=Tracker_MuonPTSignal_SelectedMCEvents

OUTFILE_NAME=$(basename $1)

TransWarpExtraction --output_file Warping_$OUTFILE_NAME --data $RECO_HIST --data_file $WARPED_FILE --data_truth $TRUE_HIST --data_truth_file $WARPED_FILE --migration Tracker_MuonPTSignal_Migration --migration_file $MIGRATION_FILE --reco $RECO_HIST --reco_file $MIGRATION_FILE --truth $TRUE_HIST --truth_file $MIGRATION_FILE --num_iter 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35 --num_uni 100 --max_chi2 100 --step_chi2 3 -P 0.22767298
