## penctl update port

update port object

### Synopsis


update port object

```
penctl update port [flags]
```

### Options

```
      --admin-state string   Set port admin state - up, down (default "up")
      --auto-neg string      Enable or disable auto-neg using enable | disable (default "enable")
      --fec-type string      Specify fec-type - rs, fc, none (default "none")
  -h, --help                 help for port
      --mtu uint32           Specify port MTU
      --num-lanes uint32     Specify number of lanes (default 4)
      --pause string         Specify pause - link, pfc, none (default "none")
      --port string          Specify port number (default "eth1/1")
      --rx-pause string      Enable or disable RX pause using enable | disable (default "enable")
      --speed string         Set port speed - none, 1g, 10g, 25g, 40g, 50g, 100g
      --tx-pause string      Enable or disable TX pause using enable | disable (default "enable")
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

