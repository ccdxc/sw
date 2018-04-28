import subprocess


class Capsim:
    def __init__(self, output):
        self.STATE = "Init"
        self.p_dict = {}
        self.r_dict = {}
        self.c_dict = {}
        self.d_dict = {}
        lines = output.split('\n')
        for line in lines:
            if self.STATE == "Init":
                if "OUTPUT STATE" in line:
                    self.STATE = "Output"
                    continue
            elif self.STATE == "Output":
                if "Combined d[]:" in line:
                    self.STATE = "D_Begin"
                elif "Combined p[]:" in line:
                    self.STATE = "P_Begin"
                elif "CFLAGS:" in line:
                    self.STATE = "C_Parse"
                continue
            elif self.STATE == "P_Begin":
                if "p = {" in line:
                    self.STATE = "P_Parse"
                    continue
            elif self.STATE == "D_Begin":
                if "d = {" in line:
                    self.STATE = "D_Parse"
                    continue
            elif self.STATE == "D_Parse":
                if "};" in line:
                    self.STATE = "Output"
                else:
                    line = line.strip(" \t;")
                    line = line.replace(" ", "")
                    key_val = line.split('=')
                    val = key_val[1].split(';')
                    self.d_dict[key_val[0]] = int(val[0], 16)
                continue
            elif self.STATE == "P_Parse":
                if "};" in line:
                    self.STATE = "R_Parse"
                else:
                    line = line.strip(" \t;")
                    line = line.replace(" ", "")
                    key_val = line.split('=')
                    val = key_val[1].split(';')
                    self.p_dict[key_val[0]] = int(val[0], 16)
                continue
            elif self.STATE == "R_Parse":
                if "r7 = " in line:
                    self.STATE = "Output"
                line = line.strip(" \t;")
                line = line.replace(" ", "")
                key_val = line.split('=')
                self.r_dict[key_val[0]] = int(key_val[1], 16)
                continue
            elif self.STATE == "C_Parse":
                if "c7 = " in line:
                    self.STATE = "Output"
                line = line.strip(" \t;")
                line = line.replace(" ", "")
                key_val = line.split('=')
                self.c_dict[key_val[0]] = int(key_val[1], 16)
                continue

def __build_ctl_file(r, k, d, include_file):
    filename = "__TMP_CTL_FILE__"
    f = open(filename, "w+")
    f.write("#include \"" + include_file + "\"\n\n")

    f.write("d = {\n")
    for key, val in d.__dict__.items():
        f.write("\t" + key + " = " + hex(val) + ";\n")
    f.write("};\n\n")

    f.write("k = {\n")
    for key, val in k.__dict__.items():
        f.write("\t" + key + " = " + hex(val) + ";\n")
    f.write("};\n\n")

    for key, val in r.__dict__.items():
        f.write(key + " = " + hex(val) + ";\n")

    f.close()
    return filename

def run_capsim(capsim_bin, cflags, bin_file, r, k, d, include_file, externs):
    ctl_file = __build_ctl_file(r, k, d, include_file)
    
    externs_str_array = []
    for k, v in externs.items():
        externs_str_array.append("-P")
        externs_str_array.append(k + "=" + str(v))
    try:
        cmd = [capsim_bin, "-c", cflags, ctl_file, bin_file] + externs_str_array
        out = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        print(e.cmd)
        print(e.output)
        return None
    run_file = "__TMP_RUN_FILE__"
    f = open(run_file, "w+")
    f.write(str(out))
    f.close()
    return Capsim(out)
