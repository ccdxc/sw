# VCHub Debug Endpoints

### URL 
`{{server}}/configs/diagnostics/v1/modules/<MODULE-ID>/Debug`

### State Debug Info

payload:
```
{
 	"query": "action",
 	"parameters": {
 	  "action": "state",
 	  "key": ORCH_CONFIG_NAME
  }
}
```

### Useg Debug Info

payload:
```
{
 	"query": "action",
 	"parameters": {
 	  "action": "useg",
 	  "key": ORCH_CONFIG_NAME,
 	  "dc": DC_NAME
  }
}
```

### Pcache Debug Info

payload:
```
{
 	"query": "action",
 	"parameters": {
 	  "action": "pcache",
 	  "key": ORCH_CONFIG_NAME,
 	  "kind": KIND // optional
  }
}
```

### Trigger Sync

payload:
```
{
 	"query": "action",
 	"parameters": {
 	  "action": "sync",
 	  "key": ORCH_CONFIG_NAME
  }
}
```

### Set Vlan space size
Note: This will only take affect for VMs that are the first VM on a host (for the liftime of the orchhub process) after this call.

payload:
```
{
 	"query": "action",
 	"parameters": {
 	  "action": "vlanSpace",
 	  "key": ORCH_CONFIG_NAME,
		"count": VLAN_COUNT
  }
}
```