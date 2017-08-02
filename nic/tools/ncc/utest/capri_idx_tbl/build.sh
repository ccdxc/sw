#!/bin/bash
rm -r capri_idx_tbl
rm C_idx.bin
../../capri-ncc.py --asm-out capri_idx_tbl.p4
../../../../capsim/gen/bin/capas -I capri_idx_tbl/asm_out/ -R -o C_idx.bin C_idx.asm
mv C_idx.bin capri_idx_tbl
