# Capview

Capview is a register access (read/write) utility.
Upon executing 'capview' on naples, an interactive prompt is returned using
which all registers can be read and modified.
There are two types of registers in the CAPRI ASIC - Scalar & Vector/Array/Table.

A scalar register may have one or more fields.

A vector register has two or more scalar registers in it.

## Usage 
```
# capview --help
capview: invalid option -- '-'
usage: capview [options] file
    -f path     capviewdb.bin path
    -d          dump database to stdout
#
# capview
> help
Commands available:
  find            - Find symbol
  fset            - Read/Modifiy Write register
  help            - Show help
  info            - Info register
  read            - Read register
  set             - Set field in edit buffer
  show            - Show edit buffer
  td              - Table dump
  write           - Write register

>
```

'read' can be used to read the registers.
Read: read <reg_name>

pp_port_p_sat_p_port_cnt_ltssm_state_changed is an example of a single field register.

```
# capview
> read pp_port_p_sat_p_port_cnt_ltssm_state_changed
0x0700085c: pp_port_p_sat_p_port_cnt_ltssm_state_changed
  val: 0xff
  Fields:
    [  7:0  ] event: 0xff
>
```
pxb_sat_tgt_rsp_err is an example of a multi field register.
```
> read pxb_sat_tgt_rsp_err
0x0719a514: pxb_sat_tgt_rsp_err
  val: 0x0
  Fields:
    [ 23:16 ] bresp_err:         0x0
    [ 15:8  ] rresp_err:         0x0
    [  7:0  ] ind_cnxt_mismatch: 0x0
```


While reading the vector register, an appropriate index has to be passed.
```
> read sge_mpu1_trace[1]
0x03690050: sge_mpu1_trace[1/0x1]
  val: 0xa4d308007000000011
  Fields:
    [  93   ] debug_generation: 0x0
    [ 92:73 ] debug_index:      0x0
    [ 72:68 ] buf_size:         0xa
    [ 67:40 ] base_addr:        0x4d30800
    [  39   ] rst:              0x0
    [  38   ] wrap:             0x1
    [  37   ] instructions:     0x1
    [  36   ] table_and_key:    0x1
    [ 35:5  ] watch_pc:         0x0
    [   4   ] watch_enable:     0x1
    [   3   ] phv_error:        0x0
    [   2   ] phv_debug:        0x0
    [   1   ] trace_enable:     0x0
    [   0   ] enable:           0x1
>
```

Write: read <reg_name>; set fld=; write
Info: info <reg_name> (describe the various parts of the output)

```
# capview
>  read sge_mpu1_trace[1]
0x03690050: sge_mpu1_trace[1/0x1]
  val: 0x3000000000
  Fields:
    [  93   ] debug_generation: 0x0
    [ 92:73 ] debug_index:      0x0
    [ 72:68 ] buf_size:         0x0
    [ 67:40 ] base_addr:        0x0
    [  39   ] rst:              0x0
    [  38   ] wrap:             0x0
    [  37   ] instructions:     0x1
    [  36   ] table_and_key:    0x1
    [ 35:5  ] watch_pc:         0x0
    [   4   ] watch_enable:     0x0
    [   3   ] phv_error:        0x0
    [   2   ] phv_debug:        0x0
    [   1   ] trace_enable:     0x0
    [   0   ] enable:           0x0
>

info pp_port_p_sat_p_port_cnt_ltssm_state_changed
0x0700085c: pp_port_p_sat_p_port_cnt_ltssm_state_changed
 Fields:
    [  7:0  ] event

set event=1
write

```


info can be used to describe a particular register.
```
> info sge_mpu1_trace
0x03690040: sge_mpu1_trace[] 4 rows
  Fields:
    [  93   ] debug_generation
    [ 92:73 ] debug_index
    [ 72:68 ] buf_size
    [ 67:40 ] base_addr
    [  39   ] rst
    [  38   ] wrap
    [  37   ] instructions
    [  36   ] table_and_key
    [ 35:5  ] watch_pc
    [   4   ] watch_enable
    [   3   ] phv_error
    [   2   ] phv_debug
    [   1   ] trace_enable
    [   0   ] enable
>
```
Write/Set:
To set a field 'read' on that register has to be done first,
followed by a 'set' on the field. 'write' will complete the
operation by pushing the value to the ASIC.
```
Example 1-
> read pp_port_p_sat_p_port_cnt_ltssm_state_changed
0x0700085c: pp_port_p_sat_p_port_cnt_ltssm_state_changed
  val: 0xff
    Fields:
        [  7:0  ] event: 0xff
> set event=1
> write
> read pp_port_p_sat_p_port_cnt_ltssm_state_changed
0x0700085c: pp_port_p_sat_p_port_cnt_ltssm_state_changed
  val: 0x1
  Fields:
    [  7:0  ] event: 0x1
>

Example 2
# capview
> read pxb_sat_tgt_rsp_err
0x0719a514: pxb_sat_tgt_rsp_err
  val: 0x0
  Fields:
    [ 23:16 ] bresp_err:         0x0
    [ 15:8  ] rresp_err:         0x0
    [  7:0  ] ind_cnxt_mismatch: 0x0
> set ind_cnxt_mismatch=1
> write
> read pxb_sat_tgt_rsp_err
0x0719a514: pxb_sat_tgt_rsp_err
  val: 0x1
  Fields:
    [ 23:16 ] bresp_err:         0x0
    [ 15:8  ] rresp_err:         0x0
    [  7:0  ] ind_cnxt_mismatch: 0x1
>
```

'find' command will search for a register of certain name.

```
> find ltssm_st_changed
pp_port_c_int_c_mac_int_enable_clear.ltssm_st_changed_enable
pp_port_c_int_c_mac_int_enable_set.ltssm_st_changed_enable
pp_port_c_int_c_mac_int_test_set.ltssm_st_changed_interrupt
pp_port_c_int_c_mac_intreg.ltssm_st_changed_interrupt
4 matches
>
```
Read: Typically all fields of a multi-word register are read atomically.

Write: Typically a register is atomically Read-Modified-Written.

'capview -f' takes path to capviewdb.bin and dumps the db.
These are the supported dbs
"/pensando/etc/capviewdb.bin",
"/platform/etc/capviewdb.bin",
"/mnt/etc/capviewdb.bin",
"capviewdb.bin"

'capview -d' dumps all the registers to stdout.

## License


## Acknowledgments

