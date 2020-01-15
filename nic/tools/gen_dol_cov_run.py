#!/usr/bin/python

import yaml
import os
import json
import sys
import subprocess
import argparse
import re

parser = argparse.ArgumentParser()
parser.add_argument('--skip-targets', dest='skip_targets',
                        nargs='*', type=str,
                        default=[], help='Skip targets')
parser.add_argument('--job-files', dest='job_files',
                        nargs='*', type=str,
                        default=None, help='List of Job ymls')

args = parser.parse_args()

DOL_JOB_PREFIX = "jobd"
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
    file = coverage_output_path + "/" + OUTPUT_FILE
    output_file = open(file, "w+")
except:
    print "Error creating output file"
    sys.exit(1)





job_cfg = {}

def __update_job_cfg(job_file):
    try:
        with open(job_file) as f:
            job = yaml.load(f)
    except:
        print "Error loading : " + job_file
        sys.exit(1)

    for target in job["targets"]:
        if target in args.skip_targets:
            continue
        job_info = job["targets"][target]["commands"]
        cov_cmd = "JOB_ID=1 IGNORE_BUILD_PIPELINE=1 make clean && " + " ".join(job_info) + " COVERAGE=1"
        job_cfg["_".join(target.split("/")[-2:])] = cov_cmd

for job_file in args.job_files:
    __update_job_cfg(job_file)

json.dump(job_cfg, output_file, indent=4)
output_file.close()
