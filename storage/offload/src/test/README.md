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
  alias
  global_params
    per_core_qdepth
    block_size
    cpu_mask
    limit_rate
    status_interval
    output_file_prefix
    output_file_suffix
    store_output_files
  crypto_keys
    key
      idx
      key1
      key2
  svc_chains
    svc_chain
      idx
      name
      input
        random
        random_len
        offset
        len
        file
        pattern
        min_block_size
        max_block_size
        block_count
      ops
        compress
          flags
          algo_type
          hdr_fmt_idx
          hdr_algo
          threshold
          threshold_delta
          output_file
          output_flags
        decompress
          flags
          algo_type
          hdr_fmt_idx
          output_file
          output_flags
        hash
          flags
          algo_type
          output_file
          output_flags
        chksum
          flags
          algo_type
          output_file
          output_flags
        encrypt
          algo_type
          key_idx
          iv_data
          output_file
          output_flags
        decrypt
          algo_type
          key_idx
          iv_data
          output_file
          output_flags
        decompact
          vvbn
          output_file
          output_flags
  tests
    test
      idx
      name
      mode
      turbo
      repeat
      batch_depth
      svc_chains
        svc_chain
          idx
          name
          input
            random
            random_len
            offset
            len
            file
            pattern
            min_block_size
            max_block_size
            block_count
          ops
            compress
              flags
              algo_type
              hdr_fmt_idx
              hdr_algo
              threshold
              threshold_delta
              output_file
              output_flags
            decompress
              flags
              algo_type
              hdr_fmt_idx
              output_file
              output_flags
            hash
              flags
              algo_type
              output_file
              output_flags
            chksum
              flags
              algo_type
              output_file
              output_flags
            encrypt
              algo_type
              key_idx
              iv_data
              output_file
              output_flags
            decrypt
              algo_type
              key_idx
              iv_data
              output_file
              output_flags
            decompact
              vvbn
              output_file
              output_flags
      validations
        data_compare
          idx
          type
          file1
          file2
          pattern
          offset
          len
        size_compare
          idx
          type
          file1
          file2
          val
        retcode_compare
          idx
          type
          svc_chain
            idx
            name
            input
              random
              random_len
              offset
              len
              file
              pattern
              min_block_size
              max_block_size
              block_count
            ops
              compress
               flags
               algo_type
               hdr_fmt_idx
               hdr_algo
               threshold
               threshold_delta
               output_file
               output_flags
              decompress
               flags
               algo_type
               hdr_fmt_idx
               output_file
               output_flags
              hash
               flags
               algo_type
               output_file
               output_flags
              chksum
               flags
               algo_type
               output_file
               output_flags
              encrypt
               algo_type
               key_idx
               iv_data
               output_file
               output_flags
              decrypt
               algo_type
               key_idx
               iv_data
               output_file
               output_flags
              decompact
               vvbn
               output_file
               output_flags
          retcode
          svc_retcodes
  cp_hdr_format
    idx
    cp_hdr_field
      type
      offset
      len
      val
  cp_hdr_mapping
    entry
      pnso_algo
      hdr_algo



