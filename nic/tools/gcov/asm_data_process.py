import os
import csv
import subprocess
import json
import pdb
import operator
from collections import defaultdict
import env as env
import utils


class Stats:
    def __init__(self, instructions = 0, cycles = 0, table_type = None):
        self.total_instructions = instructions
        self.total_cycles = cycles
        if table_type:
            self.tables = {table_type : 1}
        else:
            self.tables = {}
    
    def __add__(self, other):
        result = Stats()
        result.total_instructions = self.total_instructions + other.total_instructions
        result.total_cycles = self.total_cycles + other.total_cycles
        result.tables =  utils.combine_dicts(self.tables, other.tables, operator.add)
        return result

class StatObj(object):
    def __init__(self, parent):
        self.total_stats = Stats()
        self.cnt = 0
        self.parent = parent

    def add_stat(self, stats):
        self.total_stats = self.total_stats + stats
        self.cnt += 1
        if self.parent:
            self.parent.add_stat(stats)
        
class ProgramData(StatObj):
    
    def __init__(self, parent):
        super(ProgramData, self).__init__(parent)
    
    def Init(self, table_type, instructions, cycles):
        self.add_stat(Stats(instructions, cycles, table_type))

class StageData(StatObj):
    
    def __init__(self, parent):
        super(StageData, self).__init__(parent)
        self.pgm_data = defaultdict(lambda: ProgramData(self))
        
    def add_prgm_data(self, pgm_data):
        self.prgms.append(pgm_data)
        self.total_stats = self.total_stats + pgm_data.stats
        self.add_stat(self.total_stats)

class DirectionData(StatObj):

    def __init__(self, parent):
        self.stage_data = defaultdict(lambda: StageData(self))
        super(DirectionData, self).__init__(parent)
        
class ModuleData(StatObj):
    def __init__(self, name, feature, parent):
        self.name = name
        self.feature = feature
        self.num_of_test_cases = 0
        self.direction_data = defaultdict(lambda: DirectionData(self))
        super(ModuleData, self).__init__(parent)

    @staticmethod
    def __dump_csv_header(fp, table_keys):
        strings = ["Direction", "Stage", "Instructions", "Cycles"]
        strings.extend(sorted(table_keys))
        fp.write(",".join(strings) + "\n")
        
    @staticmethod
    def __dump_csv_line(fp, direction, stage, stat, count, table_keys):
        data_strings = [direction, stage, str(stat.total_instructions/count), \
                 str(stat.total_cycles/count)]
        for table in sorted(table_keys):
            data_strings.append(str(stat.tables.get(table, 0)/count))
        csv_out = ",".join(data_strings)
        fp.write(csv_out + "\n")

    def dump_module_stats(self, output_dir):
        filename = output_dir + "/" + self.feature + "/" + self.name + ".csv"
        fp = open(filename, "w+")
        table_keys = []
        for dir in  self.direction_data:
            table_keys += self.direction_data[dir].total_stats.tables.keys()
        table_keys = set(table_keys) 
        ModuleData.__dump_csv_header(fp, table_keys)
        for dir in self.direction_data:
            direction_data = self.direction_data[dir]
            for stage in direction_data.stage_data:
                stage_data = direction_data.stage_data[stage]
                ModuleData.__dump_csv_line(fp, dir, str(stage), 
                              stage_data.total_stats, self.num_of_test_cases, table_keys)


class TableDataMap:
    def __init__(self):
        self.by_pg_name = {}
        self.by_addr_range = utils.RangeDict()
        self._init_table_map()
        self._init_table_loader_info()

    @staticmethod
    def __get_table_map_files(directory):
        cmd="find " + directory +  " -name \*.json | grep \"table_map\""
        p = subprocess.Popen(cmd,  stdout=subprocess.PIPE, shell=True)
        output, _ = p.communicate()
        return output.split("\n")
        
    def _init_table_map(self):
        for table_data_file in TableDataMap.__get_table_map_files(env.gen_dir):
            try:
                with open(table_data_file) as data_file:    
                    table_info = json.load(data_file)
            except:
                continue
            for table in table_info["tables"]:
                self.by_pg_name[table["name"]+".bin"] = table
    
    def _init_table_loader_info(self):
        reader = csv.reader(open(env.capri_loader_conf, 'rb'))
        for row in reader:
            try:
                table = self.by_pg_name[row[0]]
            except:
                #P4 plus tables have different convention
                # Their tables names will not be found in the json files.
                continue
            table["start_addr"] = "0x" + row[1]
            table["end_addr"] = "0x" + row[2]
            range_addr = xrange(int(row[1], 16), int(row[2], 16) + 1)
            self.by_addr_range[range_addr] = table 
    

def generate_pipeline_summary_page(output_dir, page_name="instructions_summary.html"):
    os.chdir(output_dir)
    def create_html_page(index_page_name):

        header = """<!DOCTYPE html>
    <html>

    <head>
      <title>Summary</title>
    </head>

    <body>"""
        index_page = open(index_page_name, "w+")
        index_page.write(header)
        return index_page
    
    def close_html_page(index_page):
        trailer= """</body>

    </html>"""
        index_page.write(trailer + "\n")
        index_page.close()


    summary_page = create_html_page(page_name)
    asm_cov_str = "<br><strong>Features</strong></br>"
    summary_page.write(asm_cov_str)
    p4_data_path = output_dir + "/"
    for feature in next(os.walk(p4_data_path))[1]:
        index_page = p4_data_path + feature + "/index.html"
        index_page_html = create_html_page(index_page)
        os.chdir(feature)
        for csv_file_name in next(os.walk(p4_data_path + feature))[2]:
            if csv_file_name.endswith(".csv"):
                module_name = csv_file_name.split(".csv")[0]
                csv_file = p4_data_path + feature + "/" + csv_file_name
                html_file = p4_data_path + feature + "/" +  module_name + ".html"
                utils.convert_csv_to_html(csv_file, html_file,  feature + "_" + module_name)
                line = "<br><a href=%s>%s</a></br>" % (os.path.relpath(html_file, os.getcwd()), module_name)
                index_page_html.write(line)
        close_html_page(index_page_html)
        line = "<br><a href=%s>%s</a></br>" % (feature + "/index.html", feature)
        summary_page.write(line + "\n")
        os.chdir(output_dir)
        
    close_html_page(summary_page)
    os.chdir(env.nic_dir)


def generate_pipeline_data(data, asm_out_dir, output_dir):
    obj_dir_path = data["obj_dir"] + "/"
    cur_dir = os.getcwd()
    table_map = TableDataMap()
    illegal_instr_file = open(output_dir + "/" + "illegal_ins.txt", "w+")
   
    for feature in next(os.walk(asm_out_dir))[1]:
        cmd = "mkdir -p " + output_dir + "/" + feature
        subprocess.call([cmd], shell=True)
        for module_dir in next(os.walk(asm_out_dir + "/" + feature))[1]:
            module_data = ModuleData(os.path.basename(module_dir), feature, None)
            for tc_dir in next(os.walk(asm_out_dir + "/" +  feature + "/" + module_dir))[1]:
                module_data.num_of_test_cases += 1
                for tc_file in next(os.walk(asm_out_dir + "/" + feature + "/" + module_dir + "/" +  tc_dir))[2]:
                    pgm_addr = int(tc_file.split("_")[0], 16)
                    table_data = table_map.by_addr_range[pgm_addr]
                    if not table_data:
                        illegal_instr_file.write("Illegal Address : %s (%s:%s:%s)" % (hex(pgm_addr),
                                                                    module_data.name, tc_dir, tc_file) + "\n")
                        continue
                    source_file = table_data["name"]+ ".asm"
                    cmd = "find " + env.asm_src_dir + " -name " + source_file
                    p = subprocess.Popen(cmd,  stdout=subprocess.PIPE, shell=True)
                    output, _ = p.communicate()
                    output = output.split("\n")[0]
                    if output == '':
                        continue
                    dir_path = os.path.dirname(output)
                    os.chdir(dir_path)
                    
                    cano_file = table_data["name"]+ ".cano"
                    cano_file = cur_dir + "/" + obj_dir_path +  cano_file
                    cmd =  [env.capcov_cmd]
                    cmd.append("-s")
                    cmd.append(table_data["start_addr"])
                    cmd.append(asm_out_dir + "/" + feature + "/" + module_dir  + "/" + tc_dir + '/' + tc_file)
                    cmd.append(cano_file)
                    p = subprocess.Popen(cmd,  stdout=subprocess.PIPE)
                    output, _ = p.communicate()
                    for line in output.split("\n"):
                        if "instructions" in line:
                            instructions = int(line.split(";")[0].strip().split(" ")[0])
                            cycles = int(line.split(";")[1].strip().split(" ")[0])
                            direction_data = module_data.direction_data[table_data["direction"]]
                            stage_data = direction_data.stage_data[table_data["stage"]]
                            pgm_data = stage_data.pgm_data[source_file]
                            pgm_data.Init(("hbm" if table_data.get("hbm") else table_data["match_type"]),
                                          instructions, cycles)
                            break
                    else:
                        print ("Failed to execute program:", " ".join(cmd))
                        assert 0
                    os.chdir(cur_dir)
            if module_data.num_of_test_cases:
                module_data.dump_module_stats(output_dir)
