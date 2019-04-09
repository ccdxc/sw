## penctl show metrics sonic

Metrics for Storage Offload NIC

### Synopsis



---------------------------------
 Metrics for Storage Offload NIC
---------------------------------


### Options

```
  -h, --help   help for sonic
```

### Options inherited from parent commands

```
  -j, --json      display in json format (default true)
      --verbose   display penctl debug log
  -v, --version   display version of penctl
  -y, --yaml      display in yaml format
```

### SEE ALSO
* [penctl show metrics](penctl_show_metrics.md)	 - Show metrics from Naples
* [penctl show metrics sonic hw_ring](penctl_show_metrics_sonic_hw_ring.md)	 - Metrics for hardware rings
 Key indices - RId: ring ID, SubRId: sub-ring ID


Ring ID:

  cp     : compression
  cp_hot : compression (priority)
  dc     : decompression
  dc_hot : decompression (priority)
  xts_enc: XTS encryption
  xts_dec: XTS decryption
  gcm_enc: GCM encryption
  gcm_dec: GCM decryption


Value Description:

PIndex     : ring producer index
CIndex     : ring consumer index
InputBytes : total input bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)
OutputBytes: total output bytes (not available for cp_hot, dc_hot, xts_enc/dec, gcm_enc/dec)
SoftResets : number of soft resets executed
* [penctl show metrics sonic sequencer_info](penctl_show_metrics_sonic_sequencer_info.md)	 - Sequencer queues information:
 Key indices - LifId: logical interface ID, QId: queue ID


Value Description:

QStateAddr: queue state memory address
QGroup    : queue group
            0 - compress/decompress
            1 - compress/decompress status
            2 - crypto
            3 - crypto status
CoreId    : CPU core ID (not available currently)
* [penctl show metrics sonic sequencer_metrics](penctl_show_metrics_sonic_sequencer_metrics.md)	 - Metrics for sequencer queues:
 Key indices - LifId: logical interface ID, QId: queue ID


Value Description:

InterruptsRaised : CPU interrupts raised
NextDBsRung      : chaining doorbells rung
SeqDescsProcessed: sequencer descriptors processed
SeqDescsAborted  : sequencer descriptors aborted (due to reset)
StatusPdmaXfers  : status descriptors copied
HwDescXfers      : descriptors transferred to hardware
HwBatchErrors    : hardware batch (length) errors
HwOpErrors       : hardware operation errors
AolUpdateReqs    : AOL list updates requested
SglUpdateReqs    : scatter/gather list updates requested
SglPdmaXfers     : payload DMA transfers executed
SglPdmaErrors    : payload DMA errors encountered
SglPadOnlyXfers  : pad-data-only DMA transfers executed
SglPadOnlyErrors : pad-data-only DMA errors encountered
AltDescsTaken    : alternate (bypass-onfail) descriptors executed
AltBufsTaken     : alternate buffers taken
LenUpdateReqs    : length updates requested
CpHeaderUpdates  : compression header updates requested
SeqHwBytes       : bytes processed

