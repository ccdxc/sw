#!/usr/bin/python3

import argparse
import os
import sys
import subprocess
import json
import csv
import pdb
from collections import defaultdict
import asm_data_process
import env
import utils
import time

parser = argparse.ArgumentParser(description='Coverage generator')
parser.add_argument('--config', dest='conf_file',
                    default='coverage.json', help='Coverage config file')
parser.add_argument('--run-only', dest='run_only',
                    default=None, help='Run only job')
parser.add_argument('--ignore-errors', dest='ignore_errors', action='store_true', help='Ignore Dol errors and continue coverage.)')
parser.add_argument('--max-job-run-time', dest='max_job_run_time',
                    default=60, help='Maximum run time per job in minutes')
parser.add_argument('--skip-asm-ins-stats', dest='skip_asm_ins_stats', action='store_true', help='Skip ASM pipeline .)')
args = parser.parse_args()

def _get_max_job_run_time():
    return int(args.max_job_run_time) * 60

bazel_tmp_dir = env.coverage_output_path + ".bazel_tmp_out"
subprocess.call(["mkdir", "-p", bazel_tmp_dir])

gcov_out_name = "gcov_out"
FNULL = open(os.devnull, 'w')

instruction_summary_page_name = "instructions_summary.html"

feature_summary_page_name = "feature_summary.html"

def get_obj_dirs(obj_dir_type, top_dir, relative_path):

    obj_dir_map_func = {"hierarchical"   : lambda top_dir, relative_path : [top_dir + "/" + "/".join(relative_path.split("/")[1:]) + "/"],
                         "flat"  : lambda top_dir, relative_path : [top_dir + "/"],
                         "bazel" :  lambda top_dir, relative_path : [top_dir + "/" + "/".join(relative_path.split("/")[1:]) + "/"],
                         }
    return obj_dir_map_func[obj_dir_type](top_dir, relative_path)


coverage_registry = {}

def register_connector(target_class):
    coverage_registry[target_class.__name__] = target_class

class CoverageBase:
    
    _cov_types_ = {}
    
    def __init__(self):
        pass
    
    @classmethod
    def factory(cls, cov_type):
        try:
            return cls._cov_types_[cov_type]
        except KeyError:
            assert 0

    @classmethod
    def register(cls, cov_type):
        def decorator(subclass):
            cls._cov_types_[cov_type] = subclass
            return subclass
        return decorator
    
    @staticmethod
    def is_valid_coverage_info_file(file):
        pass
    
    @staticmethod
    def gen_html(info_file=None, info_dir=None, cov_output_dir=None, ignore_errors=True):
        pass

    @staticmethod
    def generate_coverage(data, name, cov_output_dir):
        pass

    @staticmethod
    def merge_coverage_files(self, test_name, lcov_info_files, output_file):
        pass
    
    @staticmethod
    def create_coverage_file(self, directory, output_file):
        pass

    @staticmethod
    def remove_files(self, lcov_out_file, remove_files):
        pass

        
@CoverageBase.register('lcov')
class LcovCoverage(CoverageBase):
    
    def __init__(self):
        pass
    
    @staticmethod
    def gcov_create(file,  obj_dir_path, dest_dir):
        subprocess.call(["gcov", file, "-o", obj_dir_path],
                        stdout=FNULL, stderr=FNULL)
        gcov_out_file = file + ".gcov"
        if dest_dir and os.path.isfile(gcov_out_file):
            subprocess.call(["mv", gcov_out_file, dest_dir])
        
    @staticmethod
    def create_coverage_file(directory, output_file):
        subprocess.call(["lcov", "-t", "result", "-o",
                         output_file, "-c", "-d", directory, "--ignore-errors", "gcov"])
    
    @staticmethod
    def is_valid_coverage_info_file(file):
        for line in open(file):
            if "SF:" in line:
                return True
        return False
    
    @staticmethod
    def gen_html(info_file=None, info_dir=None, cov_output_dir=None, ignore_errors=True):
        cmd = ["genhtml", "-o", cov_output_dir, info_file]
        if ignore_errors:
            cmd.extend(["--ignore-errors", "source"])
        subprocess.call(cmd)

    @staticmethod
    def generate_coverage(data, name, cov_output_dir):
        gcov_out_dir = cov_output_dir + "/" + gcov_out_name
        data.get("obj_dir_type")
        subprocess.call(["mkdir", "-p", gcov_out_dir])
        lcov_info_files = []
        os.chdir(env.nic_dir)
        exclude_dirs = [env.nic_dir + "/" + d for d in data.get("exclude_dirs", [])]
        
        #This is hack for now, move all the *.gcda and *.gcno files to same level.
        if data.get("obj_dir_type") == "bazel":
            #Move all the GCDA files to same location.
            os.chdir(data.get("obj_dir"))
            mv_cmd = "find . -name '*.gcda'  -type f |  xargs -i mv {} " + bazel_tmp_dir + "/" + name
            subprocess.call([mv_cmd], shell=True)
            os.chdir(env.nic_dir)
        
        for dir in data.get("dirs", []):
            for root, dirs, files in os.walk(dir, topdown=True):
                dirs[:] = [d for d in dirs if env.nic_dir  + "/" + root + "/" + d not in exclude_dirs]
                output_dir = gcov_out_dir + "/" + root
                subprocess.call(["mkdir", "-p", output_dir])
                cur_dir = os.getcwd()
                os.chdir(root)
                atleast_one_file_present = False
                gcno_ext = ".gcno"
                gcda_ext = ".gcda"
                if data.get("obj_dir_type") == "hierarchical":
                    obj_dir_path = cur_dir + "/"  + data["obj_dir"] + "/" + \
                        "/".join(root.split("/")[1:]) + "/"
                elif data.get("obj_dir_type") == "flat":
                    obj_dir_path = cur_dir + "/" + data["obj_dir"] + "/"
                elif data.get("obj_dir_type") == "bazel":
                    obj_dir_path = bazel_tmp_dir + "/" + name + "/"
                    gcno_ext = ".pic.gcno"
                    gcda_ext = ".pic.gcda"
                else:
                    assert 0
                    
                obj_dir_paths = get_obj_dirs(data.get("obj_dir_type"), data["obj_dir"], root)
                for file in files:
                    if file.endswith(tuple(data.get("file_patterns", [".c", ".cc"]))):
                        gcno_file = file.rsplit(".", 1)[0] + gcno_ext
                        gcno_file = obj_dir_path + gcno_file
                        if not os.path.isfile(gcno_file):
                            # File not compiled at all, ignore.
                            continue
                        gcda_file = file.rsplit(".", 1)[0] + gcda_ext
                        gcda_file = obj_dir_path + gcda_file
                        # File compiled but no gcda, not executed.
                        if not os.path.isfile(gcda_file):
                            # TODO Log ERROR for this.
                            continue
                            #subprocess.call(["touch", gcda_file])
                        atleast_one_file_present = True
                        subprocess.call(["cp", gcda_file, "."])
                        subprocess.call(["cp", gcno_file, "."])
                        LcovCoverage.gcov_create(file,  obj_dir_path, output_dir)

                if atleast_one_file_present:
                    lcov_out_file = output_dir + "/" + \
                        root.split("/")[-1] + "_coverage.info"
                    LcovCoverage.create_coverage_file(".", lcov_out_file)
                    if LcovCoverage.is_valid_coverage_info_file(lcov_out_file):
                        LcovCoverage.remove_files(lcov_out_file, data.get("remove_files"))
                        if LcovCoverage.is_valid_coverage_info_file(lcov_out_file):
                            lcov_info_files.append(lcov_out_file)
                    else:
                        subprocess.call(["rm", lcov_out_file])
                subprocess.call(["rm -f *.gcov *.gcno *.gcda"], shell=True)
                os.chdir(cur_dir)

        # Merge all into one.
        output_file = cov_output_dir + "/" + name + ".info"
        LcovCoverage.merge_coverage_files(name, lcov_info_files, output_file)
        LcovCoverage.gen_html(info_file=output_file, cov_output_dir=cov_output_dir)

        #Hack for now.
        if data.get("obj_dir_type") == "bazel":
            os.chdir(bazel_tmp_dir + "/" + name)
            subprocess.call(["find . -type f -name '*.gcda' -delete"], shell=True)
            os.chdir(env.nic_dir)
        return output_file

    @staticmethod
    def merge_coverage_files(test_name, lcov_info_files, output_file):
        cmd = ["lcov", "-t", test_name]
        for file in lcov_info_files:
            cmd.append("--add-tracefile")
            cmd.append(file)
        cmd.append("--output-file")
        cmd.append(output_file)
        cmd.extend(["--ignore-errors", "source"])
        subprocess.call(cmd)

    @staticmethod
    def remove_files(lcov_out_file, remove_files):
        if remove_files:
            cmd = ["lcov", "-r", lcov_out_file]
            cmd.extend(remove_files)
            cmd.extend(["-o", lcov_out_file,  "--ignore-errors", "source"])
            subprocess.call(cmd)     


@CoverageBase.register('capcov')
class CapcovCoverage(CoverageBase):
    
    def __init__(self):
        pass
    
    @staticmethod
    def create_coverage_file(directory, output_file):
        subprocess.call(["lcov", "-t", "result", "-o",
                         output_file, "-c", "-d", directory, "--ignore-errors", "gcov"])
    
    @staticmethod
    def is_valid_coverage_info_file(file):
        return True

    
    @staticmethod
    def gen_html_local(info_file=None, info_dir=None, cov_output_dir=None, ignore_errors=True):
        cwd = os.getcwd()
        os.chdir(info_dir)
        cmd = [env.gcovr_cmd, "-r",  ".", "--html", "--html-details", "-o",
                 cov_output_dir + "/" + info_file + ".html", "-g", "-k", "--gcov-ext", "cacov"]
        subprocess.call(cmd)
        os.chdir(cwd)

        
    @staticmethod
    def generate_coverage(data, name, cov_output_dir):
        entry_dir = os.getcwd()
        gcda_file = os.environ.get("MPU_COV_DUMP_FILE")
        if not gcda_file:
            print ("MPU_COV_DUMP_FILE not set")
            sys.exit(1)
        if not os.path.getsize(gcda_file):
            print ("Coverage data not generated for ", name)
            raise
        subprocess.call(["mkdir", "-p", cov_output_dir])

        try:
            reader = csv.reader(open(env.capri_loader_conf, 'r'))
        except:
            print ("Error reading capri loader configuration file")
            sys.exit(1)

        cap_loader_info = {}
        for row in reader:
            cap_loader_info[row[0]] = {"start_addr" : "0x" + row[1], "end_addr" : "0x" + row[2]}
        
        for dir in data.get("dirs", []):
            for root, dirs, files in os.walk(dir):
                obj_dir_path = data["obj_dir"] + "/"
                root_output_dir = cov_output_dir + "/" + "capcov_out" + "/" + "/".join(root.split("/")[1:])
                subprocess.call(["mkdir", "-p", root_output_dir])
                cur_dir = os.getcwd()
                os.chdir(root)
                for file in files:
                    if file.endswith(tuple(data.get("file_patterns"))):
                        bin_file = file.rsplit(".", 1)[0] + ".bin"
                        program_info = cap_loader_info.get(bin_file)
                        if not program_info:
                            continue
                        cano_file = file.rsplit(".", 1)[0] + ".cano"
                        capcov_out_file = file + ".cacov"
                        cano_file = cur_dir + "/" + obj_dir_path + cano_file
                        cmd =  [env.capcov_cmd]
                        cmd.append("-s")
                        cmd.append(program_info["start_addr"])
                        cmd.append(gcda_file)
                        cmd.append(cano_file)
                        subprocess.call(cmd, stdout=FNULL, stderr=FNULL)
                        subprocess.call(["mv", capcov_out_file, root_output_dir], stderr=FNULL)
                        #Copy Source file too to generate HTML output.
                        subprocess.call(["cp", file, root_output_dir], stderr=FNULL)
                try:
                    #Generate Directory level (Feature) coverage information.
                    if len(root.split("/")) == 2:
                        CapcovCoverage.gen_html_local(root.split("/")[1], root_output_dir, cov_output_dir)
                except:
                    pass
                os.chdir(cur_dir)
        os.chdir(entry_dir)

        #Generate HTML at top level as well.
        CapcovCoverage.gen_html_local(name, cov_output_dir, cov_output_dir)
        subprocess.call(["cp", gcda_file, cov_output_dir], stderr=FNULL)
        #Empty the current GCDA file as we are done processing.
        open(gcda_file, 'w').close()
        return cov_output_dir +  "/" + name + ".html"

    
    @staticmethod
    def merge_coverage_files(test_name, info_files, output_file):
        
        def get_cacov_file_map(dir_name):
            find_cmd = ["find", dir_name, "-name",  "*.cacov"]
            p = subprocess.Popen(find_cmd,  stdout=subprocess.PIPE)
            output, _ = p.communicate()
            files = output.decode().split("\n")
            return { os.path.basename(file) : file for file in files if file} 
   
        def merge_cacov_files(src_file_name, dst_file_name):
            tmp_file_name = "/tmp/temp_cacov_file.cacov"
            src_file = open(src_file_name, "r")
            dst_file = open(dst_file_name, "r")
            dst_tmp_file = open(tmp_file_name, "w+")
            for src_file_line, dst_file_line in zip(src_file, dst_file):
                if src_file_line.split(":")[0].strip() >  dst_file_line.split(":")[0].strip():
                    dst_tmp_file.write(src_file_line)
                else:
                    dst_tmp_file.write(dst_file_line)
                
            dst_tmp_file.close()
            dst_file.close()
            src_file.close()
            subprocess.call(["mv", tmp_file_name, dst_file_name])
        
        if not info_files:
            return 

        dst_dir_name = os.path.dirname(output_file)
        capcov_dst_dir_name = dst_dir_name + "/capov_out"
        cmd = "mkdir -p " + capcov_dst_dir_name
        subprocess.call([cmd], shell=True)
        for info_file in info_files:
            src_dir_name = os.path.dirname(info_file)
            src_dir_files_map = get_cacov_file_map(src_dir_name)
            dst_dir_files_map = get_cacov_file_map(capcov_dst_dir_name)
            #Merge the interestion of 2 files maps
            for file in (set(src_dir_files_map.keys()) & set(dst_dir_files_map.keys())):
                merge_cacov_files(src_dir_files_map[file], dst_dir_files_map[file])
            #Just copy the files if not found in the destination.
            for file in (set(src_dir_files_map.keys()) -  set(dst_dir_files_map.keys())):
                capcov_dir = src_dir_files_map[file].split("capcov_out")
                cacov_file = capcov_dir[1][1:]
                cur_dir = os.getcwd()
                os.chdir(capcov_dir[0] + "/capcov_out")
		#Copy all the files here.
		#First time when we try to merge, there is nothin in destination.
		# This will copy both cacov files and soruce file.
		#Cacov files subsequent merges and source file for html generation.
                files =  ('.').join(cacov_file.split('.')[:-1]) + "*"
                copy_cmd = "cp  --parents " + files + " " + capcov_dst_dir_name
                subprocess.call([copy_cmd], shell=True)
                os.chdir(cur_dir)
            
        os.chdir(capcov_dst_dir_name)
        for root, _, _ in os.walk("."):
            #Generate Directory level (Feature) coverage information.
            if len(root.split("/")) == 2:
                root_output_dir = capcov_dst_dir_name + "/" + "/".join(root.split("/")[1:])
                CapcovCoverage.gen_html_local(root.split("/")[1], root_output_dir, dst_dir_name)
        os.chdir(env.nic_dir)
                
        #Generate top level HTML information.
        CapcovCoverage.gen_html_local(test_name, dst_dir_name, dst_dir_name)

    @staticmethod
    def remove_files(self, lcov_out_file, remove_files):
        pass

def gen_model_bulls_eye_coverage():
    cmd = [env.bullseye_covhtml_cmd,
           "-v", "-f", os.path.realpath(env.bullseye_model_cov_file),
            env.bullseye_model_html_output_dir]
    subprocess.call(cmd)
    cmd = ["cp", os.path.realpath(env.bullseye_model_cov_file),
		env.bullseye_model_html_output_dir]
    subprocess.call(cmd)

def gen_hal_bulls_eye_coverage():
    cmd = [env.bullseye_covselect_cmd,
            "-f", os.path.realpath(env.bullseye_hal_cov_file),
            "--import", env.bullseye_hal_filter_cov_file]
    subprocess.call(cmd)
    cmd = [env.bullseye_covhtml_cmd,
           "-v", "-f", os.path.realpath(env.bullseye_hal_cov_file),
            env.bullseye_hal_html_output_dir]
    subprocess.call(cmd)
    cmd = ["cp", os.path.realpath(env.bullseye_hal_cov_file),
		env.bullseye_hal_html_output_dir]
    subprocess.call(cmd)

def gen_bulls_eye_coverage():
    gen_model_bulls_eye_coverage()
    gen_hal_bulls_eye_coverage()
 
def run_cmd(cmd, timeout=None):
    try:
        ret = subprocess.call(cmd, shell=True, timeout=timeout)
    except subprocess.TimeoutExpired:
        time.sleep(5)
        print ("Job timed out : ", cmd)
        sys.exit(1)
    if ret and not args.ignore_errors:
        print("Cmd failed.: ", cmd)
        sys.exit(1)

def build_modules(data):
    # build all modules.
    os.chdir(env.nic_dir)
    for module_name in data["modules"]:
        module_data = data["modules"][module_name]
        print ("Building module: ", module_name)
        if "clean_cmd" in module_data:
            run_cmd(module_data["clean_cmd"])
        if "build_cmd" in module_data:
            run_cmd(module_data["build_cmd"])

        #This is hack for now, move  *.gcno files to same level.
        if module_data["obj_dir_type"] == "bazel":
            gcno_dir = bazel_tmp_dir + "/" + module_name
            subprocess.call(["mkdir", "-p", gcno_dir])
            os.chdir(module_data["obj_dir"] +  "/" + os.path.basename(env.nic_dir))
            cp_cmd = "find . -name '*.gcno'  -type f |  xargs -i cp {} " + gcno_dir
            subprocess.call([cp_cmd], shell=True)
            os.chdir(env.nic_dir)
 
def run_and_generate_coverage(data):

    def generate_run_coverage(run_name, run, sub_run_name=None):
        for module_name in run[run_name]["modules"]:
            module = data["modules"][module_name]
            dir_name = "_".join([run_name, sub_run_name]) if sub_run_name else run_name
            cov_output_dir = env.coverage_output_path + dir_name + "/" + module_name
            cov_instance = CoverageBase.factory(module["cov_type"])
            try:
                module_infos[module_name].append(cov_instance.generate_coverage(
                    module, module_name, cov_output_dir))
            except Exception as ex:
                print(ex)
                print("Coverage generation failed for run :", run_name)
                #continue
            os.chdir(env.nic_dir)
            obj_dir = module.get("obj_dir")
            os.chdir(obj_dir)
            subprocess.call(["find . -type f -name '*.gcda' -delete"], shell=True)
            os.chdir(env.nic_dir)

    
    module_infos = defaultdict(lambda: [])
    for run in data["run"]:
        for run_name in run:
            if args.run_only and args.run_only != run_name:
                continue
            if "cmd" in run[run_name]:
                run_cmd(run[run_name]["cmd"], _get_max_job_run_time())
                generate_run_coverage(run_name, run)
            elif "cmd_cfg" in run[run_name]:
                with open(run[run_name]["cmd_cfg"]) as cmd_cfg_file:
                    cmd_cfg_data = json.load(cmd_cfg_file)
                    for sub_run in cmd_cfg_data:
                        run_cmd(cmd_cfg_data[sub_run], _get_max_job_run_time())
                        generate_run_coverage(run_name, run, sub_run)
            else:
                #Run not specified, may be all the meta files already generated.
                generate_run_coverage(run_name, run)

    # Finally generate lcov combined output as well.
    for module_name in module_infos:
        module = data["modules"][module_name]
        cov_output_dir = env.coverage_output_path + "/" + "total_cov" + "/" + module_name
        subprocess.call(["mkdir", "-p", cov_output_dir])
        output_file = cov_output_dir + "/total.info"
        cov_instance = CoverageBase.factory(module["cov_type"])
        cov_instance.merge_coverage_files("Total_" + module_name, 
                                          module_infos[module_name],
                                          output_file)
        cov_instance.gen_html(output_file, cov_output_dir=cov_output_dir)


def generate_coverage_summary_page(cov_output_dir, page_name="coverage_summary.html"):
    os.chdir(cov_output_dir)

    header = """<!DOCTYPE html>
<html>

<head>
  <title>Code Coverage Information</title>
</head>

<body>"""
    op_file = open(page_name, "w+")
    op_file.write(header)
    total_cov_info = """
    <strong>Total Coverage</strong>
    <p class="indent": 5em>
    <br><a href="total_cov/hal/index.html">HAL Total Coverage</a></br>
    <br><a href="%s">BullsEye HAL Total Coverage</a></br>
    <br><a href="total_cov/model/index.html">Model Total Coverage</a></br>
    <br><a href="%s">BullsEye Model Total Coverage</a></br>
    <br><a href="total_cov/asm/Total_asm.html">ASM Total Coverage</a></br>
    </p>
    """ %((os.path.relpath(env.bullseye_hal_html_output_dir) + "/index.html"),
	(os.path.relpath(env.bullseye_model_html_output_dir) + "/index.html"))

    op_file.write(total_cov_info)
    asm_cov_str = "<br><br><strong>ASM Detailed Information</strong></br></br>"
    op_file.write(asm_cov_str)
    for root, dirs, files in os.walk("total_cov/asm", topdown=True):
        for file in files:
            if ".html" in file and len(file.split(".")) == 2:
                line = "&nbsp <a href=%s>%s</a> &nbsp;" % (root + "/" + file, file.split(".html")[0])
                op_file.write(line)

    instructions_summary_page = env.p4_data_output_path + "/" + instruction_summary_page_name
    line = "<br><br><a href=%s>%s</a></br> </br>" % (os.path.relpath(instructions_summary_page, os.getcwd()),
                                                "ASM Detailed Instructions Statistics")
    op_file.write(line)
    feature_summary_page = env.p4_data_output_path + "/" + feature_summary_page_name
    line = "<a href=%s>%s</a>" % (os.path.relpath(feature_summary_page, os.getcwd()),
                                                "ASM Feature Instructions Statistics")
    op_file.write(line)
    
    trailer= """</body>

</html>"""
    op_file.write(trailer + "\n")
    op_file.close()
    os.chdir(env.nic_dir)


if __name__ == '__main__':
    config_file = env.coverage_path + args.conf_file
    if not os.path.isfile(config_file):
        print ("Config file %s found" % (config_file))
        sys.exit(1)

    with open(config_file) as data_file:
        data = json.load(data_file)

    os.chdir(env.nic_dir)
    build_modules(data)
    run_and_generate_coverage(data)
    subprocess.call(["mkdir", "-p", env.p4_data_output_path])
    for module_name in data["modules"]:
        if not args.skip_asm_ins_stats and data["modules"][module_name]["cov_type"] == "capcov":
            asm_data_process.generate_pipeline_data(data["modules"][module_name], 
                                    env.asm_out_final, env.p4_data_output_path)
            asm_data_process.generate_pipeline_summary_page(env.p4_data_output_path,
                                                             instruction_summary_page_name)
            asm_data_process.generate_feature_sub_stats(env.p4_data_output_path,
                                                        feature_summary_page_name)
    gen_bulls_eye_coverage()
    generate_coverage_summary_page(env.coverage_output_path)

