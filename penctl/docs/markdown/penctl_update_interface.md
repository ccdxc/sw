## penctl update interface

Create interface

### Synopsis


Create interface

```
penctl update interface [flags]
```

### Options

```
      --egress-bw uint32        Specify egress bandwidth in KBytes/sec <0-12500000 KBytes/sec>. 0 means no policer
      --encap string            Encap type (Ex: MPLSoUDP)
      --gw-mac string           Specify gateway MAC address as aabb.ccdd.eeff
  -h, --help                    help for interface
      --ingress-bw uint32       Specify ingress bandwidth in KBytes/sec <0-12500000 KBytes/sec>. 0 means no policer
      --mpls-in string          Specify incoming MPLS label as comma separated list (Max of 2 supported)
      --mpls-out uint32         Specify outgoing MPLS label
      --name string             Interface name
      --overlay-ip string       Specify overlay IPv4 address in comma separated list (Max of 2 supported). Ex: 1.2.3.4,2.3.4.5
      --overlay-mac string      Specify overlay MAC address as aabb.ccdd.eeff (optional)
      --pf-mac string           Specify PF MAC address as aabb.ccdd.eeff (optional)
      --source-gw string        Specify source gateway. Must be IPv4 prefix as a.b.c.d/nn
      --substrate-ip string     Substrate IPv4 address
      --tunnel-dest-ip string   Tunnel destination IPv4 address
```

### Options inherited from parent commands

```
  -a, --authtoken string   path to file containing authorization token
      --compat-1.1         run in 1.1 firmware compatibility mode
      --dsc-url string     set url for Distributed Service Card
  -j, --json               display in json format (default true)
      --verbose            display penctl debug log
  -v, --version            display version of penctl
  -y, --yaml               display in yaml format
```

### SEE ALSO
* [penctl update](penctl_update.md)	 - Update Object

