#!/usr/bin/python

import yaml
import os
import json
import sys
import subprocess

DOL_JOB_PREFIX = "./run.py"
DOL_COVERAGE_OPTION = "--coveragerun"
OUTPUT_FILE = "dol_cov_runs.json"

nic_dir = os.environ.get("NIC_DIR")
if not nic_dir:
    print "Nic not set, please set NIC_DIR"
    sys.exit(1)
nic_dir = os.path.abspath(nic_dir)

coverage_output_path = os.environ.get("COVERAGE_OUTPUT")
if not coverage_output_path:
    print "Coverage output not set, please set COVERAGE_OUTPUT"
    sys.exit(1)
coverage_output_path = os.path.abspath(coverage_output_path) + "/"

os.chdir(nic_dir)

try:
    with open(".job.yml") as f:
        job = yaml.load(f)
except:
    print "Error loading .job.yml"
    sys.exit(1)

subprocess.call(["mkdir", "-p", coverage_output_path])

try:
    file = coverage_output_path + "/" + OUTPUT_FILE
    output_file = open(file, "w+")
except:
    print "Error creating output file"
    sys.exit(1)

job_cfg = {}
for target in job["targets"]:
    job_info = job["targets"][target]
    for cmd in job_info:
    	if DOL_JOB_PREFIX in cmd:
            cov_cmd = " ".join([DOL_JOB_PREFIX , DOL_COVERAGE_OPTION,
				 cmd.split(DOL_JOB_PREFIX, 1)[1]])
            job_cfg[target] = cov_cmd

json.dump(job_cfg, output_file, indent=4)
output_file.close()
