## Vsimctl - command line interface for VC simulator

Vsimctl uses VCSim's REST API to create and delete VMs.

Usage:
```
host1:~$ vsimctl
Needs -create or -delete flag
  -create
    	Create VM
  -delete
    	Delete VM
  -hostif string
    	Host facing interface (default "ntrunk0")
  -ip string
    	VM ip address
  -mac string
    	VM mac address
  -net string
    	VM Network (default "default")
  -vlan uint
    	VM useg vlan
  -vm string
    	VM Name (default "vm1")

```


Example:
```
vsimctl -create -hostif eth2 -ip 10.1.1.5/24 -net default -vlan 25
```
