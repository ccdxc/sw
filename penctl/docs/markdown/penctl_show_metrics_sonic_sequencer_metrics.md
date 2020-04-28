## penctl show metrics sonic sequencer_metrics

Metrics for sequencer queues

### Synopsis



---------------------------------
 Metrics for sequencer queues:
 Key indices - LifId: logical interface ID, QId: queue ID


Value Description:


InterruptsRaised	: CPU interrupts raised
NextDBsRung	: chaining doorbells rung
SeqDescsProcessed	: sequencer descriptors processed
SeqDescsAborted	: sequencer descriptors aborted (due to reset)
StatusPdmaXfers	: status descriptors copied
HwDescXfers	: descriptors transferred to hardware
HwBatchErrors	: hardware batch (length) errors
HwOpErrors	: hardware operation errors
AolUpdateReqs	: AOL list updates requested
SglUpdateReqs	: scatter/gather list updates requested
SglPdmaXfers	: payload DMA transfers executed
SglPdmaErrors	: payload DMA errors encountered
SglPadOnlyXfers	: pad-data-only DMA transfers executed
SglPadOnlyErrors	: pad-data-only DMA errors encountered
AltDescsTaken	: alternate (bypass-onfail) descriptors executed
AltBufsTaken	: alternate buffers taken
LenUpdateReqs	: length updates requested
CpHeaderUpdates	: compression header updates requested
SeqHwBytes	: bytes processed

---------------------------------


```
penctl show metrics sonic sequencer_metrics [flags]
```

### Options

```
  -h, --help   help for sequencer_metrics
```

### Options inherited from parent commands

```
  -a, --authtoken string   path to file containing authorization token
      --compat-1.1         run in 1.1 firmware compatibility mode
      --dsc-url string     set url for Distributed Service Card
  -j, --json               display in json format (default true)
  -t, --tabular            display in table format
      --verbose            display penctl debug log
  -v, --version            display version of penctl
  -y, --yaml               display in yaml format
```

### SEE ALSO
* [penctl show metrics sonic](penctl_show_metrics_sonic.md)	 - Metrics for Storage Offload DSC

