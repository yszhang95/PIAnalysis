#!/bin/bash
njobs=10
# python3 submit_TargetSim.py \
#         --outpath /data/eliza5/PIONEER/yousen/pienu_atar_only_doubleside_analysis/ \
#         --container /data/eliza5/PIONEER/containers/pioneer_latest.sif \
#         --compile-path /home/yousen/production-Mar-2024/main/ \
#         --njobs $njobs \
#         --geometry atar_only_doubleside_20240307.gdml \
#         --required-files run_pienu.C run_pimue.C pienu_filelists.txt pimue_filelists.txt PIAnalysis.tar.gz \
#         --post-process-script run_pienu.sh \
#         --do-simulation 0

python3 submit_TargetSim.py \
        --outpath /data/eliza5/PIONEER/yousen/pimue_atar_only_doubleside_analysis/ \
        --container /data/eliza5/PIONEER/containers/pioneer_latest.sif \
        --compile-path /home/yousen/production-Mar-2024/main/ \
        --njobs $njobs \
        --geometry atar_only_doubleside_20240307.gdml \
        --required-files run_pienu.C run_pimue.C pienu_filelists.txt pimue_filelists.txt PIAnalysis.tar.gz \
        --post-process-script run_pimue.sh \
        --do-simulation 0
