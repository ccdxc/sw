## penctl update naples

Set NAPLES Modes and Profiles

### Synopsis



----------------------------
 Set NAPLES configuration 
----------------------------


```
penctl update naples [flags]
```

### Options

```
  -c, --controllers stringSlice     List of controller IP addresses or hostnames
  -g, --default-gw string           Default GW for mgmt
  -d, --dns-servers stringSlice     List of DNS servers
  -h, --help                        help for naples
  -n, --hostname string             Host name
  -o, --managed-by string           NAPLES Management. host or network (default "host")
  -k, --management-network string   Management Network. inband or oob
  -m, --mgmt-ip string              Management IP in CIDR format
  -f, --naples-profile string       Active NAPLES Profile (default "default")
  -p, --primary-mac string          Primary mac
```

### Options inherited from parent commands

```
  -j, --json      display in json format (default true)
      --verbose   display penctl debug log
  -v, --version   display version of penctl
  -y, --yaml      display in yaml format
```

### SEE ALSO
* [penctl update](penctl_update.md)	 - Update Object

