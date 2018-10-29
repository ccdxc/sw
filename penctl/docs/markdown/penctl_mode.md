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
  -c, --controllers stringSlice   List of controller IP addresses or hostnames
  -g, --default-gw string         Default GW for mgmt
  -d, --dns-servers stringSlice   List of DNS servers
  -h, --help                      help for mode
  -n, --hostname string           Host name
  -i, --mgmt-ip string            Management IP in CIDR format
  -m, --mode string               host or network managed (default "network")
  -p, --primary-mac string        Primary mac
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

