# PenCAKE
## What is PenCAKE?
Pensando Compression Accelerator and Key Encryptor
is a tool for testing validity and performance of
the storage offload driver sonic.ko.  The APIs
exported by sonic.ko (and consumed by pencake.ko)
are all defined in pnso_api.h.

PenCAKE takes YAML configuration files as input,
and outputs YAML summary status for each testcase.

## Using pencake_sim on host (no Naples required)
```
1. copy sonic tarball to host
2. ssh to host andssh myhost
2. compile it: cd storage/offload; bazel build ...
2. see command line options available: pencake_sim -h
3. Run e.g.: pencake_sim -p /sw/storage/offload/src/test/ -i 4k.yml globals.yml cpdc.yml
```

## Using pencake sanity on Naples host
```
1. lspci | grep 1007
2. cd storage/offload ; make modules
3. (optional help) modinfo sonic.ko; modinfo pencake.ko
4. insmod ./sonic.ko core_count=4
5. insmod ./pencake.ko cpu_mask=0xf repeat=1000000 batch=16
6. dmesg -w | grep PenCAKE
7. cat /sys/module/pencake/status/*
```

## Using pencake custom tests on Naples host
```
1. lspci | grep 1007
2. cd storage/offload ; make modules
3. (optional help) modinfo sonic.ko; modinfo pencake.ko
4. insmod ./sonic.ko core_count=4
5. insmod ./pencake.ko cpu_mask=0xf feat_mask=0
6. (example) cp src/test/4k.yml /sys/module/pencake/cfg/0
7. (example) cp src/test/globals.yml /sys/module/pencake/cfg/1
8. (example) cp src/test/cpdc.yml /sys/module/pencake/cfg/2
9. echo start > /sys/module/pencake/cfg/ctl
10. cat /sys/module/pencake/status/*
```

## YAML structure
When in doubt, refer to storage/offload/include/pnso_api.h or
the sample YAML config files in storage/offload/src/test/*.yml.
For detailed reference, the implementation of YAML parsing can be
found in pnso_test_parser.c.

Valid YAML structure:
  alias_group1
    alias
  alias_group2
    alias
  alias_group3
    alias
  alias_group4
    alias
  alias_group5
    alias
  global_params
    store_output_files
    output_file_suffix
    output_file_prefix
    status_interval
    limit_rate
    cpu_mask
    block_size
    per_core_qdepth
  crypto_keys
    key
      key2
      key1
      idx
  svc_chains
    svc_chain
      ops
        decompact
          output_flags
          output_file
          vvbn
        decrypt
          output_flags
          output_file
          iv_data
          key_idx
          algo_type
        encrypt
          output_flags
          output_file
          iv_data
          key_idx
          algo_type
        chksum
          output_flags
          output_file
          algo_type
          flags
        hash
          output_flags
          output_file
          algo_type
          flags
        decompress
          output_flags
          output_file
          hdr_fmt_idx
          algo_type
          flags
        compress
          output_flags
          output_file
          threshold_delta
          threshold
          hdr_algo
          hdr_fmt_idx
          algo_type
          flags
      input
        block_count
        max_block_size
        min_block_size
        pattern
        file
        len
        offset
        random_len
        random
      name
      idx
  tests
    test
      validations
        retcode_compare
          svc_retcodes
          retcode
          svc_chain
          type
          idx
        size_compare
          val
          file2
          file1
          type
          idx
        data_compare
          len
          offset
          pattern
          file2
          file1
          type
          idx
      svc_chains
      batch_depth
      repeat
      turbo
      mode
      name
      idx
  cp_hdr_formats
    format
      cp_hdr_fields
        field
          val
          len
          offset
          type
      idx
  cp_hdr_mapping
    entry
      hdr_algo
      pnso_algo
