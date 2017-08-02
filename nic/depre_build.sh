#!/bin/bash

if [ $# -gt 1 ] ; then
     echo "Usage: $0 [--clean]"
     exit
fi


declare -a cmd_arr=("cd utils && make -j8 all"
                    "cd fte && make -j8 all"
                    "cd hal/pd/capri/mrl && make -j8 all"
                    "cd proto && make -j8 all"
                    "cd p4/nw && make -j8 all"
                    "cd capsim && make -j8"
                    "cd asm/nw && make -j8 all"
                    "cd hal/pd/iris && make -j8 all"
                    "cd model_sim/build && make all"
                    "cd hal/pd/capri && make -j8 all"
                    "cd hal/pd/utils && make -j8 all"
                    "cd hal && make -j8 all"
                    "cd hal/test && make -j8 all"
                    "cd agents && make -j8 all"
                    "cd sknobs_gen/ && ./sknobs_gen.sh"
                    )

declare -a clean_arr=("cd utils && make clean"
                      "cd fte && make clean"
                      "cd hal/pd/iris && make clean"
                      "cd hal/pd/capri/mrl && make clean"
                      "cd hal/pd/capri && make clean"
                      "cd hal && make clean"
                      "cd proto && make clean"
                      "cd agents && make clean"
                      "cd capsim && make clean"
                      "rm -rf gen/*.o"
                      "cd p4/nw && make clean"
                      "cd asm/nw && make clean"
                      )

function exec_cmd()
{
    echo "Running cmd: $1"
    if  eval "$1" ; then
        return 0
    else
        echo "Failed execution of $1"
        exit 1
    fi
}

function run_make()
{
    param=("${!1}")
    root_dir=$PWD
    for cmd in "${param[@]}"
    do
        exec_cmd "$cmd"
        eval "cd $root_dir"
    done
}

if [ "$1" != "" ]; then
    if [ "$1" == "--clean" ]; then
        run_make clean_arr[@]
    else
        echo "Invalid option $1"
        echo "Usage: $0 [--clean]"
        exit
    fi
else
    run_make cmd_arr[@]
fi
