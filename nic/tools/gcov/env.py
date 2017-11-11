import sys
import os

coverage_path = os.environ.get("COVERAGE_CONFIG_PATH")
if not coverage_path:
    print ("Coverage config path not set, please set COVERAGE_CONFIG_PATH")
    sys.exit(1)
coverage_path = os.path.abspath(coverage_path) + "/"

coverage_output_path = os.environ.get("COVERAGE_OUTPUT")
if not coverage_output_path:
    print ("Coverage output not set, please set COVERAGE_OUTPUT")
    sys.exit(1)
coverage_output_path = os.path.abspath(coverage_output_path) + "/"
asm_out_final = coverage_output_path + "asm_out_final"
p4_data_output_path = coverage_output_path + "p4_data"

nic_dir = os.environ.get("NIC_DIR")
if not nic_dir:
    print ("Nic not set, please set NIC_DIR")
    sys.exit(1)
nic_dir = os.path.abspath(nic_dir)
asm_src_dir = nic_dir + "/" + "asm"

capcov_cmd = os.environ.get("CAPCOV")
if not capcov_cmd:
    print ("Capcov command path not set, please set CAPCOV")
    sys.exit(1)

gcovr_cmd = os.environ.get("GCOVR")
if not gcovr_cmd:
    print ("Gcvor command path not set, please set GCOVR")
    sys.exit(1)
    
capri_loader_conf = os.environ.get("CAPRI_LOADER_CONF")
if not capri_loader_conf:
    print ("Capri loader conf path not set, please set CAPRI_LOADER_CONF")
    sys.exit(1)
    
gen_dir = os.environ.get("GEN_DIR")
if not gen_dir:
    print ("GEN Dir path not set, please set GEN_DIR")
    sys.exit(1)    
