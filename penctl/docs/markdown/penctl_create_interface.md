## penctl create interface

Create interface

### Synopsis


Create interface

```
penctl create interface [flags]
```

### Options

```
      --egress-bw uint32        Specify egress bandwidth in Kbytes/sec
      --encap string            Encap type (Ex: MPLSoUDP)
      --gw-mac string           Specify gateway MAC address as aabb.ccdd.eeff
  -h, --help                    help for interface
      --ingress-bw uint32       Specify ingress bandwidth in Kbytes/sec
      --mpls-in string          Specify incoming MPLS label as comma separated list (Max 2 supported
      --mpls-out uint32         Specify outgoing MPLS label
      --name string             Interface name
      --overlay-ip string       Specify overlay IPv4 address in comma separated list (Max of 2 supported). Ex: 1.2.3.4,2.3.4.5
      --source-gw string        Specify source gateway. Must be IPv4 prefix as a.b.c.d/nn
      --substrate-ip string     Substrate IPv4 address
      --tunnel-dest-ip string   Tunnel destination IPv4 address
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
* [penctl create](penctl_create.md)	 - Create Object

