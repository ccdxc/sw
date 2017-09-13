#!/usr/bin/python

import argparse
import os
import sys
import subprocess
import json
from collections import defaultdict

parser = argparse.ArgumentParser(description='Coverage generator')
parser.add_argument('--config', dest='conf_file',
                    default='coverage.json', help='Coverage config file')
args = parser.parse_args()

coverage_path = os.environ.get("COVERAGE_CONFIG_PATH")
if not coverage_path:
    print "Coverage config path not set, please set COVERAGE_CONFIG_PATH"
    sys.exit(1)
coverage_path = os.path.abspath(coverage_path) + "/"

coverage_output_path = os.environ.get("COVERAGE_OUTPUT")
if not coverage_output_path:
    print "Coverage output not set, please set COVERAGE_OUTPUT"
    sys.exit(1)
coverage_output_path = os.path.abspath(coverage_output_path) + "/"

obj_dir_path = os.environ.get("OBJ_DIR")
if not obj_dir_path:
    print "Obj dir not set, please set OBJ_DIR"
    sys.exit(1)
obj_dir_path = os.path.abspath(obj_dir_path) + "/"


nic_dir = os.environ.get("DIR_NIC")
if not nic_dir:
    print "Nic not set, please set DIR_NIC"
    sys.exit(1)
nic_dir = os.path.dirname(nic_dir)
nic_dir = os.path.abspath(nic_dir)


top_dir = os.getcwd()
gcov_out_name = "gcov_out"
FNULL = open(os.devnull, 'w')


def is_valid_coverage_info_file(file):
    for line in open(file):
        if "SF:" in line:
            return True
    return False


def gen_html(lcov_info_file, cov_output_dir):
    cmd = ["genhtml", "-o", cov_output_dir, lcov_info_file]
    subprocess.call(cmd)


def gcov_create(file, dest_dir=None):
    subprocess.call(["gcov", file, "-o", obj_dir_path],
                    stdout=FNULL, stderr=FNULL)
    gcov_out_file = file + ".gcov"
    if dest_dir and os.path.isfile(gcov_out_file):
        subprocess.call(["mv", gcov_out_file, dest_dir])


def lcov_create(dir, lcov_out_file):
    subprocess.call(["lcov", "-t", "result", "-o",
                     lcov_out_file, "-c", "-d", dir, "--ignore-errors", "gcov"])


def lcov_remove(lcov_out_file, remove_files):
    if remove_files:
        cmd = ["lcov", "-r", lcov_out_file]
        cmd.extend(remove_files)
        cmd.extend(["-o", lcov_out_file,  "--ignore-errors", "source"])
        subprocess.call(cmd)


def generate_coverage(data, test_name,  cov_output_dir):
    gcov_out_dir = cov_output_dir + "/" + gcov_out_name
    subprocess.call(["mkdir", "-p", gcov_out_dir])
    lcov_info_files = []
    for dir in data.get("nic_dirs", []):
        for root, dirs, files in os.walk(nic_dir + "/" + dir):
            output_dir = gcov_out_dir + "/" + root
            subprocess.call(["mkdir", "-p", output_dir])
            os.chdir(root)
            atleast_one_file_present = False
            for file in files:
                if file.endswith(tuple(data.get("file_patterns", [".c", ".cc"]))):
                    gcno_file = file.rsplit(".", 1)[0] + ".gcno"
                    if not os.path.isfile(obj_dir_path + gcno_file):
                        # File not compiled at all, ignore.
                        continue
                    gcda_file = file.rsplit(".", 1)[0] + ".gcda"
                    # File compiled but not gcda, not executed.
                    if not os.path.isfile(obj_dir_path + gcda_file):
                        # continue
                        subprocess.call(["touch", obj_dir_path + gcda_file])
                    atleast_one_file_present = True
                    subprocess.call(["cp", obj_dir_path + gcda_file, "."])
                    subprocess.call(["cp", obj_dir_path + gcno_file, "."])
                    gcov_create(file,  output_dir)

            if atleast_one_file_present:
                lcov_out_file = output_dir + "/" + \
                    root.split("/")[-1] + "_coverage.info"
                lcov_create(".", lcov_out_file)
                if is_valid_coverage_info_file(lcov_out_file):
                    lcov_remove(lcov_out_file, data.get("remove_files"))
                    lcov_info_files.append(lcov_out_file)
                else:
                    subprocess.call(["rm", lcov_out_file])
            subprocess.call(["rm -f *.gcov *.gcno *.gcda"], shell=True)
            os.chdir(top_dir)

    # Merge all into one.
    output_file = cov_output_dir + "/" + test_name + ".info"
    merge_lcov_files(test_name, lcov_info_files, output_file)
    gen_html(output_file, cov_output_dir)
    return output_file
    # for file in lcov_info_files:
    #    subprocess.call(["rm", file])


def merge_lcov_files(test_name, lcov_info_files, output_file):
    cmd = ["lcov", "-t", test_name]
    for file in lcov_info_files:
        cmd.append("--add-tracefile")
        cmd.append(file)
    cmd.append("--output-file")
    cmd.append(output_file)
    subprocess.call(cmd)


def run_test(cmd):
    os.chdir(nic_dir)
    ret = subprocess.call(cmd, shell=True)
    if ret:
        print cmd, " failed."
        sys.exit(1)
    os.chdir(top_dir)


def build(cmd):
    os.chdir(nic_dir)
    ret = subprocess.call("make clean", shell=True)
    if ret:
        print("Clean failed.")
        sys.exit(1)
    ret = subprocess.call(cmd, shell=True)
    if ret:
        print("Build failed.")
        sys.exit(1)
    os.chdir(top_dir)


if __name__ == '__main__':
    config_file = coverage_path + args.conf_file
    if not os.path.isfile(config_file):
        print "Config file %s found" % (config_file)
        sys.exit(1)

    with open(config_file) as data_file:
        data = json.load(data_file)

    # First do a build.
    build(data["make_cmd"])

    lcov_info_files = []
    for test in data["cov_tests"]:
        run_test(test["cmd"])
        lcov_info_files.append(generate_coverage(
            data,  test["name"], coverage_output_path + test["output_dir_name"]))
        os.chdir(obj_dir_path)
        subprocess.call(["rm -f *.gcda"], shell=True)
        os.chdir(top_dir)

    # Finally generate lcov combined output as well.
    cov_output_dir = coverage_output_path + "/" + "total_cov"
    subprocess.call(["mkdir", "-p", cov_output_dir])
    output_file = cov_output_dir + "/total.info"
    merge_lcov_files("Total", lcov_info_files, output_file)
    gen_html(output_file, cov_output_dir)
