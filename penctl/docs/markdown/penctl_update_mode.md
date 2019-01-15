## penctl update mode

Set Naples to Managed mode

### Synopsis



----------------------------
 Set NAPLES management mode 
----------------------------


```
penctl update mode [flags]
```

### Options

```
  -c, --controllers stringSlice   List of controller IP addresses or hostnames
  -g, --default-gw string         Default GW for mgmt
  -d, --dns-servers stringSlice   List of DNS servers
  -h, --help                      help for mode
  -n, --hostname string           Host name
  -o, --management-mode string    host or network managed (default "network")
  -m, --mgmt-ip string            Management IP in CIDR format
  -k, --network-mode string       oob or inband (default "oob")
  -p, --primary-mac string        Primary mac
```

### Options inherited from parent commands

```
  -i, --interface string   ethernet device of naples
  -j, --json               display in json format
  -t, --tabular            display in tabular format (default true)
  -v, --version            display version of penctl
  -y, --yaml               display in yaml format
```

### SEE ALSO
* [penctl update](penctl_update.md)	 - Update Object

