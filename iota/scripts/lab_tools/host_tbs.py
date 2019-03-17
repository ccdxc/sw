#! /usr/bin/python3
import sys
import os

top_dir = os.path.dirname(sys.argv[0])
sys.path.insert(0,'%s' % top_dir)
sys.path.insert(0,'../../')
import harness.infra.utils.parser as par
import re
from threading import Thread
from multiprocessing.dummy import Pool as ThreadPool 

POS_HOSTNAME = 0
POS_OS = 1

def __print(msg, _to_screen=False):
    if _to_screen:
        print (msg)

def __get_testbeds(filename):
    tbs = []
    with open(filename, "r") as fp:
        for line in fp:
            print (line)
            x = re.search("(tb\d{1,2})", line)
            if x != None:
                tbs.append(x.group(0))
            pass
    __print (tbs)
    return tbs

def __file_exist(filename):
    return os.path.isfile(filename)

def __get_tb_json(tbname):
    filename = "/vol/jenkins/iota/testbeds/%s.json" % tbname
    if __file_exist(filename):
        return filename
    filename = "/vol/jenkins/iota/dev-testbeds/%s.json" % tbname
    if __file_exist(filename):
        return filename
    return None

def __get_tb_json_map(tbs):
    ret_dict = {}
    for tb in tbs:
        json_file = __get_tb_json(tb)
        if json_file == None:
            continue
        ret_dict[tb] = json_file
    __print (ret_dict)
    return ret_dict

def __get_testbed_object(filename):
    return par.JsonParse(filename).Instances

def __get_naples_in_tb(json_file):
    ret_list = []
    insts = __get_testbed_object(json_file)
    for inst in insts:
        if inst.Resource.NICType == "pensando":
            ret_list.append((inst.NodeMgmtIP, inst.NodeOs))
    __print (ret_list)
    return ret_list

def __get_tb_naples_map(tb_json_map):
    ret_dict = {}
    for tb in tb_json_map:
        ret_dict[tb] = __get_naples_in_tb(tb_json_map[tb])
    __print (ret_dict)
    return ret_dict

def __os_run(task):
    print (task[1])
    op = os.popen(task[1]).read()
    print ( "[%s] %s" % (task[0], op))
    return ("[%s]\n %s\n" % (task[0], op))

def __run_cmd(tbs_naples_map, command, ostorunon):
    run_parallel = True
    def __build_and_run_commands(tbs_naples_map, command):
        tasks = []
        for tb in tbs_naples_map:
            for naples in tbs_naples_map[tb]:
                hostname = naples[POS_HOSTNAME]
                osname   = naples[POS_OS]
                if ostorunon and osname in ostorunon:
                    tasks.append((hostname, "python host_script.py --hostname %s --command \"%s\"\n" % (hostname, command )))
    
        results = []
        if run_parallel:
            pool = ThreadPool(len(tasks))
            results = pool.map(__os_run, tasks)
        else:
            for task in tasks:
                results.append(__os_run(task))
        return results
    
    def __print_results(results):
        print ("\n------------------------------------\n")
        for result in results:
            print ("\n----------\n")
            print (result)
            print ("\n----------\n")
        print ("\n------------------------------------\n")

    results = __build_and_run_commands(tbs_naples_map, command)
    __print_results( results)
    return results

import argparse
parser = argparse.ArgumentParser(description='Testbeds check script')
parser.add_argument('--filename', dest='filename', required = False, default=True, help='print frequency file')
parser.add_argument('--testbed', dest='testbed', required = False, default=False, help='print frequency file')
parser.add_argument('--command', dest='command', required = True, default=True, help='print frequency file')
parser.add_argument('--os-types', dest='os_types', required = False, default=True, help='print frequency file')
GlobalOptions = parser.parse_args()

def Main():
    #read local file to get list of tbs
    if not GlobalOptions.testbed:
        tbs = __get_testbeds(GlobalOptions.filename)
    else:
        tbs = [GlobalOptions.testbed]
    # get tesbed json files
    tbs_json_map = __get_tb_json_map(tbs)
    tbs_naples_map = __get_tb_naples_map(tbs_json_map)
    # Now here construct command to run where frequency.json is enabled
    __run_cmd(tbs_naples_map,GlobalOptions.command, GlobalOptions.os_types.split(','))

if __name__ == '__main__':
    try: 
        Main()
    except Exception as ex:
        sys.stderr.write(str(ex))
        sys.exit(1)
    sys.exit(os.EX_OK)
