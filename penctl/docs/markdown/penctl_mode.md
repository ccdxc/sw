## penctl mode

Set Naples to Managed mode

### Synopsis



-----------------------------------
 Set Naples to Venice Managed mode 
-----------------------------------


```
penctl mode [flags]
```

### Options

```
  -c, --cluster-ip stringSlice   List of cluster IP addresses (Required)
  -h, --help                     help for mode
  -n, --host-name string         Host name (Required)
  -i, --management-ip string     Management IP (Required)
  -m, --mode string              unmanaged vs managed mode (default "managed")
  -p, --primary-mac string       Primary mac (Required)
```

### Options inherited from parent commands

```
  -e, --interface string   ethernet device of naples
  -j, --json               display in json format
  -t, --tabular            display in tabular format (default true)
  -v, --verbose            verbose output
  -y, --yaml               display in yaml json
```

### SEE ALSO
* [penctl](penctl.md)	 - Pensando CLIs
* [penctl mode detail](penctl_mode_detail.md)	 - Show mode of operation of Naples

