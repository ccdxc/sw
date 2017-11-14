#!/bin/bash

OPTS=`getopt -o t::pur --long txt_file::,pack,unpack,run -n 'package.sh' -- "$@"`

# Checking if getopt failed
if [ $? != 0 ] ; then 
    echo "Failed parsing options." >&2 ; 
    echo $?
    exit 1 ; 
fi 

echo "$OPTS"
echo "The number of arguments is: $#"
eval set -- "$OPTS"

PACK=false
UNPACK=false
RUN_HAL=false

# Parsing Options
while true; do
    case "$1" in
        -t|--txt_file )
            case "$2" in
                "") TXT_FILE='nic/pack_files.txt' ; shift 2 ;;
                *) TXT_FILE=$2 ; shift 2 ;;
            esac ;;
        -p|--pack ) PACK=true; shift ;;
        -u|--unpack ) UNPACK=true; shift ;;
        -r|--run ) RUN_HAL=true; shift ;;
        -- ) shift; break ;;
        * ) break ;;
    esac
done

# Checking if we got an unknown option
if [ $# -ne 0 ]; then
    echo "Unknown Option(s): $@"
    exit 1
fi

# Input
echo PACK=$PACK
echo UNPACK=$UNPACK
echo RUN_HAL=$RUN_HAL

# -----------------------------------------------------------------------------
# Packing
# -----------------------------------------------------------------------------
if [ $PACK == true ]; then
    echo "Packing ... "

    if [[ -z "${WS_DIR}" ]]; then
        echo "Fail: Have to set WS_DIR"
        exit 1
    fi

    dir_tars=()
    file_tars=()

    # Read sources to pack
    #sources=( `awk '{ print $1 }' nic/pack_files.txt `)
    sources=( `awk '{ print $1 }' ${TXT_FILE} `)
    for f in "${sources[@]}"
    do 
        echo $f
        if [[ -d $f ]]; then
            tmp_tar=$f
            tmp_tar+=".tgz"
            tar -hcvzf $tmp_tar -C $f .
            dir_tars+=($tmp_tar)
        elif [[ -f $f ]]; then
            file_tars+=($f)
        else
            echo "$f is invalid file"
            exit 1
        fi

    done
    dests=( `awk '{ print $2 }' ${TXT_FILE} `)

    #echo "files: ${files[@]}"

    # Assumption: First 2 lines are package.sh & pack_files.txt
    #temp_files=("${files[@]:0:2}")
    #temp_files+=${file_tars[@]}

    echo "files to be tarred: ${file_tars[@]}"
    echo "dirs to be tarred: ${dir_tars[@]}"
    tar -cvzf hal.tgz "${file_tars[@]}" "${dir_tars[@]}"

    #libs=("${files[@]:2}")
    #echo "Temp files: ${temp_files[@]}"
    #echo "libs: ${libs[@]}"
    #echo "dests: ${dests[@]}"
    #echo "Packing hal libs ... ${libs[@]}"
    #tar -cvzf hal_libs.tgz "${libs[@]}"
    #tar -cvzf hal.tgz hal_libs.tgz "${temp_files[@]}"
fi

# -----------------------------------------------------------------------------
# UnPacking
# -----------------------------------------------------------------------------
if [ $UNPACK == true ]; then
    echo "Unpacking ... "

    tar -xvzf hal_libs.tgz

    files=( `awk '{ print $1 }' ${TXT_FILE} `)
    dests=( `awk '{ print $2 }' ${TXT_FILE} `)
    srcs=("${files[@]:2}")

    if [ ${#srcs[@]} -ne ${#dests[@]} ]; then
        echo "ERROR: some srcs doesnt have destinations: \
            srcs:${#srcs[@]} dests: ${#dests[@]}"
        exit 1
    fi

    for ((i=0;i<${#srcs[@]};++i)); do
        # Create dest. dir
        mkdir -p ${dests[i]}
        echo "Moving ${srcs[i]} => ${dests[i]}"
        tmp=${srcs[i]}
        tmp+=".tgz"
        if [ -f ${srcs[i]} ]; then
            mv ${srcs[i]} ${dests[i]}
        elif [ -f $tmp ]; then
            tar -xvzf $tmp -C ${dests[i]}
        fi
    done

fi

# -----------------------------------------------------------------------------
# Run HAL
# -----------------------------------------------------------------------------
if [ $RUN_HAL == true ]; then
    echo "Running HAL ... "

    files=( `awk '{ print $1 }' ${TXT_FILE} `)
    dests=( `awk '{ print $2 }' ${TXT_FILE} `)
    libs=("${files[@]:2}")

    for ((i=0;i<${#libs[@]};++i)); do
        fname=`basename "${libs[i]}"`
        echo "filename: $fname"
        if [ $fname == "hal" ]; then
            echo "Hal destination: ${dests[i]}"
            hal_dest=${dests[i]}/
        fi
        if [ $fname == "hal.json" ]; then
            echo "Json destination: ${dests[i]}"
            hal_json_dest=${dests[i]}/
        fi
    done
    hal_bin=$hal_dest
    hal_bin+="hal"
    echo "Hal binary: $hal_bin"
    json=$hal_json_dest
    json+="hal.json"
    echo "json: $json"

    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/tmp/libs/
    export HAL_CONFIG_PATH=/tmp/conf
    $hal_bin -c hal.json 2>&1  
fi

