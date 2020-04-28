## penctl show metrics system memory

System memory information

### Synopsis



---------------------------------
 System memory information:


Value Description:

Total Memory: Total Memory of the system.
Available Memory: Available Memory of the system.
Free Memory: Free Memory of the system.
The memory is KB

Totalmemory	: Total memory of the system
Availablememory	: Available memory of the system
Freememory	: Free memory of the system

---------------------------------


```
penctl show metrics system memory [flags]
```

### Options

```
  -h, --help   help for memory
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
* [penctl show metrics system](penctl_show_metrics_system.md)	 - Metrics for system monitors

