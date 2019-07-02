## penctl update naples-profile

naples profile object

### Synopsis



----------------------------
 Update NAPLES Profiles 
----------------------------


```
penctl update naples-profile [flags]
```

### Options

```
  -h, --help                  help for naples-profile
  -n, --name string           Name of the NAPLES profile to be created
  -i, --num-lifs int32        Maximum number of LIFs on the eth device. 1 or 16 (default 1)
  -p, --port-default string   Set default port admin state for next reboot. (enable | disable) (default "enable")
```

### Options inherited from parent commands

```
  -a, --authtoken string   path to file containing authorization token
  -j, --json               display in json format (default true)
      --verbose            display penctl debug log
  -v, --version            display version of penctl
  -y, --yaml               display in yaml format
```

### SEE ALSO
* [penctl update](penctl_update.md)	 - Update Object

