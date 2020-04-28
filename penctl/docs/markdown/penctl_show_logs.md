## penctl show logs

Show logs from Distributed Service Card

### Synopsis



------------------------------
 Show Module Logs From Distributed Service Card 
------------------------------


```
penctl show logs [flags]
```

### Options

```
  -h, --help            help for logs
  -m, --module string   Module to show logs for
			Valid modules are:
				nmd
				netagent
				tmagent
				pciemgrd

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
* [penctl show](penctl_show.md)	 - Show Object and Information

