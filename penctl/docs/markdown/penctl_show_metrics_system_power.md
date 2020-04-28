## penctl show metrics system power

System power information

### Synopsis



---------------------------------
 System power information:


Value Description:

pin: Input power to the system.
pout1: Core output power.
pout2: Arm output power.
The power is milli Watt

Pin	: Input power to the system
Pout1	: Core output power
Pout2	: ARM output power

---------------------------------


```
penctl show metrics system power [flags]
```

### Options

```
  -h, --help   help for power
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

