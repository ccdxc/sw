Capri-Non-Compiler-Compiler (capri-ncc)
=======================================
This is capri-configuration utility that converts a P4 program into an initial
configuration for Capri asic as well as runtime API definition for table 
programming.

This utility performs the following functions for P4 program:
* PHV allocation for ingress and egress
* PHV optimization using header and field unions (via pragmas)
* Parse graph calculations, state split, OHI assignment, hv_bit assignments
* Internal states and headers to handle intrinsic and bridged metadata
* Generate parser register, TCAM and SRAM configurations
* Generate Table Engine configuration
* Generate Deprarser configuration

This utility also generates header files that can be used by MPU action 
functions written in assembly. The header files are generated in asm_out directory.
* phv_structs are generated in INGRESS/EGRESS_p.h
* k_structs are generated for each table in file INGRESS/EGRESS_<table_name>_k.h
    * The K structure includes all the match fields and all input fields used by
      various action routines of that table
* d_structs are generated for each table in a file INGRESS/EGRESS_<table_name>_d.h
    Since d_struct can be different for each action, to save the number of bits in
    the table entry, a separate d_struct is created with name -
    <table_name>_<action_name>_d {}
    All d structs are created in a single header file

Run-time API:
=============
-TBD

Requirements:
=============
* p4-hlir package and its dependencies
* Enum34 (if using python version < 3.4). It can be installed as -
    sudo pip install enum34


Examples:
=========
capri-ncc.py --asm-out ./nicp4/v2/nic.p4

Usage:
======
./capri-ncc/capri-ncc.py -h
usage: capri-ncc.py [-h] [--log LOGLEVEL] [--flog FLOGLEVEL] [--no-ohi]
                    [--asm-out] [--p4-plus]
                    [source [source ...]]

optional arguments:
  -h, --help        show this help message and exit
  --log LOGLEVEL    Set console logging level (DEBUG, INFO, WARNING, ERROR,
                    CRITICAL
  --flog FLOGLEVEL  Set File logging level (DEBUG, INFO, WARNING, ERROR,
                    CRITICAL
  --no-ohi          Do not use OHI - everything in phvs (for testing)
  --asm-out         Generate header files for capri-assembler
  --p4-plus         Compile P4+ program
