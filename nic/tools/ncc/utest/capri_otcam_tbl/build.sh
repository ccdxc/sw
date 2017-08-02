#!/bin/bash
rm -r capri_otcam_tbl
rm A_hash.bin
../../capri-ncc.py --asm-out capri_otcam_tbl.p4
../../../../capsim/gen/bin/capas -I capri_otcam_tbl/asm_out/ -R -o A_hash.bin A_hash.asm
mv A_hash.bin capri_otcam_tbl
