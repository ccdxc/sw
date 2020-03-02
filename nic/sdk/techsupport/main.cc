//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <getopt.h>

#include "ts.hpp"

using namespace sdk::ts;

techsupport_spec_t g_ts_spec;

static void
ts_create_output_dir (std::string output_dir)
{
    std::string cmd;
    std::string base_cmd = "mkdir -p ";

    cmd = base_cmd + output_dir;
    auto rc = system(cmd.c_str());
    if (rc) {
        fprintf(stderr, "Failed to run cmd %s, err %d\n",
                cmd.c_str(), rc);
        exit(1);
    }
}

static void inline
ts_print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c|--cmdfile <techsupport.json> "
            "-d|--destdir <absolute_path_for_output_directory> "
            "-o|--outfile <techsupport_file_name.gz> "
            "-s|--skipcore\n", argv[0]);
}

void
ts_process_cmd_line_args (int argc, char **argv)
{
    int oc;
    std::string ts_file;
    struct option longopts[] = {{"cmdfile",  required_argument, NULL, 'c'},
                                {"destdir",  required_argument, NULL, 'd'},
                                {"outfile",  required_argument, NULL, 'f'},
                                {"skipcore", no_argument,       NULL, 's'},
                                {"help",     no_argument,       NULL, 'h'},
                                {0,          0,                 0,     0 }};

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hsc:d:o:W;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            if (optarg) {
                g_ts_spec.cmd_file = std::string(optarg);
                if (access(g_ts_spec.cmd_file.c_str(), R_OK) < 0) {
                    fprintf(stderr, "%s doesn't exist or inaccessible\n",
                            g_ts_spec.cmd_file.c_str());
                    exit(1);
                }
            } else {
                fprintf(stderr, "command file is not specified\n");
                ts_print_usage(argv);
                exit(1);
            }
            break;

        case 'd':
            if (optarg) {
                g_ts_spec.dst_dir = std::string(optarg);
                ts_create_output_dir(g_ts_spec.dst_dir);
            } else {
                fprintf(stderr, "destination directory is not specified\n");
                ts_print_usage(argv);
                exit(1);
            }
            break;

        case 'o':
            if (optarg) {
                g_ts_spec.ts_file = std::string(optarg);
            } else {
                fprintf(stderr, "output gz file name is not specified\n");
                ts_print_usage(argv);
                exit(1);
            }
            break;

        case 's':
            g_ts_spec.skip_core = true;
            break;

        case 'h':
            ts_print_usage(argv);
            exit(0);
            break;

        case ':':
            fprintf(stderr, "%s: option -%c requires an argument\n",
                    argv[0], optopt);
            ts_print_usage(argv);
            exit(1);
            break;

        case '?':
        default:
            fprintf(stderr, "%s: option -%c is invalid, quitting ...\n",
                    argv[0], optopt);
            ts_print_usage(argv);
            exit(1);
            break;
        }
    }

    if (argc < 7) {
        fprintf(stderr, "%s: missing mandatory cmd line args\n", argv[0]);
        ts_print_usage(argv);
        exit(1);
    }

    // check if file already exists
    ts_file = g_ts_spec.dst_dir + "/" + g_ts_spec.ts_file;
    if (access(ts_file.c_str(), R_OK) == 0) {
        fprintf(stderr, "%s already exists\n", ts_file.c_str());
        exit(1);
    }
}

int
main (int argc, char **argv)
{
    int ret;
    techsupport *ts_inst;

    ts_process_cmd_line_args(argc, argv);
    ts_inst = techsupport::factory(g_ts_spec);
    ret = ts_inst->collect_techsupport();
    if (ret) {
        fprintf(stderr, "Errors while running some of the tasks in %s\n",
                g_ts_spec.cmd_file.c_str());
    } else {
        fprintf(stdout, "techsupport created in %s\n",
                g_ts_spec.ts_file.c_str());
    }
    techsupport::destroy(ts_inst);

    return 0;
}
