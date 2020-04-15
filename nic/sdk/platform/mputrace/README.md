# MPUTRACE

captrace is a tool to enable tracing on each Match Processing Unit (MPU) using
the independent trace facility provided by the ASIC.

captrace supports the following operations
config <cfg.json>
    It takes a config json file as input. The input file specifies the MPUs that
    need to be enabled for tracing.
show
    Displays the contents of trace registers of all the MPUs that have been
    enabled for tracing.
reset
    Clears the contents of trace registers of all the MPUs and stops the
    tracing.
dump <dump_file>
    Collects the traced logs from HBM into the specified binary file.

In HBM, a region is carved out for storing traced logs. Each MPU is assigned
a portion of this region based on the requested trace size. If the traced logs
exceed the trace size, option is provided to wrap around or stop the tracing.

Trace header is traced by default and contains the pipeline number, stage
number and mpu number along with other trace options provided by the user.
Along with trace header, options can be specified to capture the instructions
and key and table pairs.

For the specified MPUs in the configuration file, trace is enabled for all
packets. This can be controlled by setting flags in phv or by adding a trace
instruction in the p4 program or by watching the pc address using a watch-pc
address.

The traced logs from the HBM are copied into a binary file when the user intends
to decode. A separate utility, captrace.py is provided to decode the binary file
offline.

When MPU trace is enabled, performance will degrade due to the HBM trace
bandwidth load. When an MPU executes a trace instruction, tracing will be
enabled for that MPU until the end of the current program is reached.

## Usage :
1. Conf:
```
    # captrace conf /tmp/captrace_cfg.json
```


    A sample json is explained below. More json samples are provided in
    /nic/conf/captrace/ on the naples.

```
 {
   // 'instances' contains the list of configuration objects.

  "instances": [
    {

       // name of the pipeline - txdma/rxdma/p4ig/p4eg
       // A standard regular expression can be used here
       // for eg., ".*" to match all, "*dma" to match txdma and rxdma, etc.,
       // Default value is ".*"

      "pipeline": "txdma",

       // stage number in the pipeline (1 - 8 for txdma/rxdma and 1 - 6 for
       // ingress/egress)
       // A standard regular expression can be used here
       // "." or ".*" to match all the stages, 1-5 to specify stage 1 to
       // stage 5 inclusive
       // Default value is ".*"

      "stage": "1",

       // mpu - mpu number in the stage (1 - 4)
       // A standard regular expression can be used here
       // "." or ".*" to match all the stages, 1-5 to specify stage 1 to
       // stage 5 inclusive
       // Default value is ".*"

      "mpu": "1",

       // control - this json object contains the following boolean fields

      "control": {

         // trace - start tracing if trace instruction is executed by the
         // pipeline. Once tracing is enabled it remains enabled till end
         // of the current program is reached.

        "trace": false,

         // phv-debug - trace if phv-debug flag is set in the PHV

        "phv-debug": true,

         // phv-error - trace if phv-error flag is set in the PHV
         // once phv-debug or phv-error flag is set, then it remains enabled
         // in that MPU it is disabled

        "phv-error": true

      },

       // capture - this json object contains the following boolean fields

      "capture": {

         // key-data - if this is enabled both key and data values are traced.

        "key-data": true,

         // instructions - if this is enabled the instructions are traced

        "instructions": true

      },

       // settings - this json object contains the following fields

      "settings": {

         // trace-size - size of the trace buffer, 1024 to 8192. Default is 4096

        "trace-size": "8192",

         // wrap - If the wrap is set, then the trace will wrap back to the start
         // of the trace buffer when the trace index reaches the trace buffer
         // size. If the wrap is not set, the trace will stop and optionally
         // interrupt ARM.

        "wrap": true

      },

       // description - this field can be used to label the current json
       // object based on what it does. This is ignored by the captrace tool.

      "description": "this enables tracing for txdma pipeline, stage 1 and mpu
                      1 with the specified options"

    }
```

    All boolean flags are false by default.

    If any field or json sub block is not specified then default values are
    assumed for those.
Create a captrace_cfg.json file on NAPLES device specifying the MPUs that
require to be traced.
If two json objects have the same pipeline, stage and mpu combination, then
the former takes effect while the latter is ignored and a warning is shown to
the user.

Following is a sample config.json file for reference -
```
{
  "instances": [
    {
      "pipeline": "txdma",
      "stage": "1",
      "mpu": "1",
      "control": {
        "trace": false,
        "phv-debug": true,
        "phv-error": true
      },
      "capture": {
        "key-data": true,
        "instructions": true
      },
      "settings": {
        "trace-size": "8192",
        "wrap": true
      },
      "description": "this enables tracing for txdma pipeline, stage 1 and mpu 1 with the specified options"
    },
    {
      "pipeline": "rxdma",
      "stage": "1",
      "mpu": "1",
      "control": {
        "trace": true,
        "phv-debug": true,
        "phv-error": true
      },
      "capture": {
        "key-data": true,
        "instructions": true
      },
      "settings": {
        "trace-size": "8192",
        "wrap": true
      },
      "description": "this enables tracing for rxdma pipeline, stage 1 and mpu 1 with the specified options"
    },
    {
      "pipeline": "p4ig",
      "stage": ".",
      "mpu": ".",
      "control": {
        "trace": true,
        "phv-debug": true,
        "phv-error": true
      },
      "capture": {
        "key-data": true,
        "instructions": true
      },
      "settings": {
        "trace-size": "1024",
        "wrap": true
      },
      "description": "this enables tracing for p4 ingress pipeline, all mpus in all stages with the specified options"
    },
    {
      "pipeline": "p4ig",
      "stage": "2",
      "mpu": "2",
      "control": {
        "trace": true,
        "phv-debug": true,
        "phv-error": true
      },
      "capture": {
        "key-data": true,
        "instructions": true
      },
      "settings": {
        "trace-size": "1024",
        "wrap": true
      },
      "description": "this enables tracing for p4 ingress pipeline, stage 2, "
                     "mpu 2 which tries to overwrite the options specified in "
                     "previous object. This obj is ignored and a warning is "
                     "thrown to the user"
    }
  ]
}
```


2. show
    'captrace show'
    This will show the state of each MPU with tracing enable.

    For eg.,
```
    # captrace show

     pipeline      stage        mpu     enable       wrap      reset      trace  phv_debug  phv_error   watch_pc   table_kd      instr trace_addr trace_nent   trace_sz
         1          0          0          1          1          0          0          0          0 0x00000000          1          1 0x13e690000       1024      65536
         1          0          1          1          1          0          0          0          0 0x00000000          1          1 0x13e6a0000       1024      65536
         1          0          2          1          1          0          0          0          0 0x00000000          1          1 0x13e6b0000       1024      65536
         1          0          3          1          1          0          0          0          0 0x00000000          1          1 0x13e6c0000       1024      65536
         ...
```

3. reset
    'captrace reset'
    This will reset the tracing registers for all MPUs.
```
    # captrace reset
```

4. dump
    'captrace dump <file path>'
    This will dump the contents from HBM (High Bandwidth Memory) into the
    specified file. The file needs to be copied out and decoded to get
    the actual instructions executed by the MPUs.
```
    # captrace dump /tmp/captrace.bin

    # ls -l /tmp/captrace.bin
    -rw-r--r--    1 root     root       7347200 Jan  5 06:03 /tmp/captrace.bin
```

## Steps to decode the dump file in the container -
    1 Copy the following files into the container
        - the dump file from 'captrace dump' command
        - /nic/conf/gen/mpu_prog_info.json file from the naples device
    2 Run captrace.py script on the binary with mpu_prog_info.conf and
      captrace.syms files
        To decode the trace dump file
            sdk/platform/mputrace/captrace.py decode captrace.bin --load=mpu_prog_info.json --sym=captrace.syms
        To track packet with PHV timestamp “0x1c07a80c” across stages
            sdk/platform/mputrace/captrace.py decode captrace.bin --fltr phv_timestamp_capture=0x1c07a80c --load=mpu_prog_info.json --sym=captrace.syms > pkt1c07.log
        To dump info about the packet
            grep -e pipeline -e stage -e PROGRAM -e BRANCH -e table_hit pkt1c07.log

For some reason, if you can't find captrace.syms in your workspace you can generate it by doing:

        - cd /sw/nic/
        - sdk/platform/mputrace/captrace.py gen_syms --pipeline=<pipeline>
            - <pipeline> can be iris or apollo or artemis or gft
        - this will generate captrace.syms in nic/

5. captrace_collect.py
    captrace_collect.py is a simple tool that automates the above steps to collect and decode.
    It can connect to NAPLES either via oob_mnic0 or through the host,
    enable the trace options provided, collect the logs, decode and filter them.
    The decoded output is collected in nic/ dir.
    Usage:
        - sdk/platform/mputrace/captrace_collect.py --mgmt <naples mgmt ip> --rxdma
        or
        - sdk/platform/mputrace/captrace_collect.py --host <host ip> --rxdma
