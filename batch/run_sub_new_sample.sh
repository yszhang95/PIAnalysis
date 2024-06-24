#!/bin/bash
njobs=150

python3 submit_TargetSim.py \
        --outpath /data/eliza5/PIONEER/yousen/pimue_atar_only_doubleside_analysis_batch/ \
        --container /data/eliza5/PIONEER/containers/pioneer_latest.sif \
        --compile-path /home/yousen/production-Mar-2024/main/ \
        --njobs $njobs \
        --geometry atar_only_doubleside_20240307.gdml \
        --mac pimue.mac \
        --required-files run_pimue.C utils.C \
        --setup-options h \
        --post-process-script run_pimue.sh \
        --pass-directory-to-script 1 \
        --do-simulation 1

# python3 submit_TargetSim.py \
#         --outpath /data/eliza5/PIONEER/yousen/pienu_atar_only_doubleside_analysis_batch/ \
#         --container /data/eliza5/PIONEER/containers/pioneer_latest.sif \
#         --compile-path /home/yousen/production-Mar-2024/main/ \
#         --njobs $njobs \
#         --geometry atar_only_doubleside_20240307.gdml \
#         --mac pienu.mac \
#         --required-files run_pienu.C utils.C \
#         --setup-options h \
#         --post-process-script run_pienu.sh \
#         --pass-directory-to-script 1 \
#         --do-simulation 1
