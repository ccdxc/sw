## penctl set mode

Set Naples to Managed mode

### Synopsis



-----------------------------------
 Set Naples to Venice Managed mode 
-----------------------------------


```
penctl set mode [flags]
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
  -y, --yaml               display in yaml format
```

### SEE ALSO
* [penctl set](penctl_set.md)	 - Config CLIs

