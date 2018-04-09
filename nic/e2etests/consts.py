import os
import sys

nic_e2e_dir = os.path.dirname(sys.argv[0])
nic_dir = nic_e2e_dir + "/../"
tests_dir = "tests"
specs_dir = tests_dir + "/specs"
modules_dir = tests_dir + "/modules"
nic_dir = os.path.abspath(nic_dir)
namespaces = []

print("NIC_DIR = ", nic_dir)
if nic_dir is None:
    print("NIC dir is not set!")
    sys.exit(1)
    
HNTAP_CFG_FILE = nic_dir + "/conf/hntap.conf"
DOL_CFG_FILE = nic_dir + "/conf/dol.conf"
hntap_log = nic_dir + "/hntap.log"
E2E_APP_CONFIG_FILE = nic_dir + "/e2etests/e2e.json"