//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <getopt.h>
#include <sys/stat.h>

#include "ts.hpp"

using namespace sdk::ts;

techsupport_spec_t g_ts_spec;

static void inline
ts_print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c|--cmdfile <techsupport.json> "
            "-d|--destdir <absolute_path_for_output_directory> "
            "-o|--outfile <techsupport_file_name.gz>\n", argv[0]);
}

void
ts_process_cmd_line_args (int argc, char **argv)
{
    int oc;
    std::string output_dir;
    struct stat stat_buffer;
    struct option longopts[] = {{"cmdfile", required_argument, NULL, 'c'},
                                {"destdir", required_argument, NULL, 'd'},
                                {"outfile", required_argument, NULL, 'f'},
                                {"help",    no_argument,       NULL, 'h'},
                                {0,         0,                 0,     0 }};

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hc:d:o:W;", longopts, NULL)) != -1) {
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
                output_dir = std::string(optarg);
                if (stat(output_dir.c_str(), &stat_buffer) != 0) {
                    fprintf(stderr, "%s does NOT exist\n",
                            output_dir.c_str());
                    exit(1);
                }
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
    g_ts_spec.ts_file = output_dir + "/" + g_ts_spec.ts_file;
    if (access(g_ts_spec.ts_file.c_str(), R_OK) == 0) {
        fprintf(stderr, "%s already exists\n", g_ts_spec.ts_file.c_str());
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
    ret = ts_inst->run_tasks();
    if (ret) {
        fprintf(stderr, "Errors while running some of the tasks in %s\n",
                g_ts_spec.cmd_file.c_str());
    } else {
        fprintf(stdout, "techsupport created in %s\n",
                g_ts_spec.ts_file.c_str());
    }
    techsupport::destroy(ts_inst);

    return ret;
}
