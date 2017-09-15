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


def gcov_create(file,  obj_dir_path, dest_dir):
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


def generate_coverage(data, name, cov_output_dir):
    gcov_out_dir = cov_output_dir + "/" + gcov_out_name
    assert data.get("gcno_dir") or data.get("obj_dir")
    subprocess.call(["mkdir", "-p", gcov_out_dir])
    lcov_info_files = []
    os.chdir(nic_dir)
    for dir in data.get("dirs", []):
        for root, dirs, files in os.walk(dir):
            output_dir = gcov_out_dir + "/" + root
            subprocess.call(["mkdir", "-p", output_dir])
            cur_dir = os.getcwd()
            os.chdir(root)
            atleast_one_file_present = False
            if data.get("obj_dir"):
                obj_dir_path = data["obj_dir"] + "/" + \
                    "/".join(root.split("/")[1:]) + "/"
            else:
                obj_dir_path = data["gcno_dir"] + "/"
            for file in files:
                if file.endswith(tuple(data.get("file_patterns", [".c", ".cc"]))):
                    gcno_file = file.rsplit(".", 1)[0] + ".gcno"
                    gcno_file = cur_dir + "/" + obj_dir_path + gcno_file
                    if not os.path.isfile(gcno_file):
                        # File not compiled at all, ignore.
                        continue
                    gcda_file = file.rsplit(".", 1)[0] + ".gcda"
                    gcda_file = cur_dir + "/" + obj_dir_path + gcda_file
                    # File compiled but no gcda, not executed.
                    if not os.path.isfile(gcda_file):
                        # TODO Log ERROR for this.
                        continue
                        #subprocess.call(["touch", gcda_file])
                    atleast_one_file_present = True
                    subprocess.call(["cp", gcda_file, "."])
                    subprocess.call(["cp", gcno_file, "."])
                    gcov_create(file,  obj_dir_path, output_dir)

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
            os.chdir(cur_dir)

    # Merge all into one.
    output_file = cov_output_dir + "/" + name + ".info"
    merge_lcov_files(name, lcov_info_files, output_file)
    gen_html(output_file, cov_output_dir)
    os.chdir(top_dir)
    return output_file


def merge_lcov_files(test_name, lcov_info_files, output_file):
    cmd = ["lcov", "-t", test_name]
    for file in lcov_info_files:
        cmd.append("--add-tracefile")
        cmd.append(file)
    cmd.append("--output-file")
    cmd.append(output_file)
    subprocess.call(cmd)


def run(cmd):
    os.chdir(nic_dir)
    ret = subprocess.call(cmd, shell=True)
    if ret:
        print("Cmd failed.: ", cmd)
        sys.exit(1)
    os.chdir(top_dir)


if __name__ == '__main__':
    config_file = coverage_path + args.conf_file
    if not os.path.isfile(config_file):
        print "Config file %s found" % (config_file)
        sys.exit(1)

    with open(config_file) as data_file:
        data = json.load(data_file)

    # build all modules.
    for module_name in data["modules"]:
        print "Building module: ", module_name
        run(data["modules"][module_name]["clean_cmd"])
        run(data["modules"][module_name]["build_cmd"])
        module = data["modules"][module_name]

    lcov_info_files = []
    module_infos = defaultdict(lambda: [])
    for run_name in data["run"]:
        run(data["run"][run_name]["cmd"])
        for module_name in data["run"][run_name]["modules"]:
            module = data["modules"][module_name]
            module_infos[module_name].append(generate_coverage(
                module, module_name, coverage_output_path + run_name + "/" + module_name))
            if module.get("gcno_dir"):
                os.chdir(module["gcno_dir"])
                subprocess.call(["rm -f *.gcda"], shell=True)
                os.chdir(top_dir)

    # Finally generate lcov combined output as well.
    for module_name in module_infos:
        cov_output_dir = coverage_output_path + "/" + "total_cov" + "/" + module_name
        subprocess.call(["mkdir", "-p", cov_output_dir])
        output_file = cov_output_dir + "/total.info"
        merge_lcov_files("Total " + module_name,
                         module_infos[module_name], output_file)
        gen_html(output_file, cov_output_dir)
