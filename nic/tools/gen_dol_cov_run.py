#!/usr/bin/python

import yaml
import os
import json
import sys
import subprocess
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--skip-targets', dest='skip_targets',
                        nargs='*', type=str,
                        default=[], help='Skip targets')
args = parser.parse_args()

DOL_JOB_PREFIX = "./run.py"
DOL_COVERAGE_OPTION = "--coveragerun"
MODEL_LOG_OPTION = "--modellogs"
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

model_logging_enabled = os.environ.get("MODEL_LOGGING_ENABLED")

os.chdir(nic_dir)

try:
    with open(".job.yml") as f:
        job = yaml.load(f)
except:
    print "Error loading .job.yml"
    sys.exit(1)

try:
    file = coverage_output_path + "/" + OUTPUT_FILE
    output_file = open(file, "w+")
except:
    print "Error creating output file"
    sys.exit(1)

job_cfg = {}
for target in job["targets"]:
    if target in args.skip_targets:
        continue
    job_info = job["targets"][target]
    for cmd in job_info:
    	if DOL_JOB_PREFIX in cmd:
            run_args = cmd.split(DOL_JOB_PREFIX, 1)[1].split("&&")[0]
	    options = [DOL_JOB_PREFIX, DOL_COVERAGE_OPTION, run_args]
            if model_logging_enabled:
	        options.append(MODEL_LOG_OPTION)
            cov_cmd = " ".join(options)
            job_cfg[target] = cov_cmd

json.dump(job_cfg, output_file, indent=4)
output_file.close()
