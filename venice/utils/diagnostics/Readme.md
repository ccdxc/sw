## Corvo Diagnostics Framework

Corvo can be used to get diagnostics information from running process or container.  Each process or container is represented by a Module object. It is currently created in API server by CMD for venice containers.
Currently, any Venice grpc server can include Corvo service. It includes an implementation for a handler to query logs. One can extend it to define new query and its handler to return custom diagnostics. Examples could be debug stats, profiling information etc.
For an example integration you can look at Spyglass. One can set a log level in module object for Spyglass and query its logs using Debug action.



![Architecture](./corvo.jpg "Architecture")



### Module
**Request**
```
GET https://{{VeniceServer}}:{{VenicePort}}/configs/diagnostics/v1/modules/node1-pen-spyglass
```
**Response**
```
{
    "kind": "Module",
    "api-version": "v1",
    "meta": {
        "name": "node1-pen-spyglass",
        "generation-id": "2",
        "resource-version": "666",
        "uuid": "ce378940-8718-4473-a391-75f9b8461293",
        "creation-time": "2019-05-10T16:26:10.112345874Z",
        "mod-time": "2019-05-10T16:26:27.176632168Z",
        "self-link": "/configs/diagnostics/v1/modules/node1-pen-spyglass"
    },
    "spec": {
        "log-level": "Debug"
    },
    "status": {
        "node": "node1",
        "module": "pen-spyglass",
        "category": "Venice",
        "service": "pen-spyglass-2044869589-3v2dt"
    }
}
```
### Log query on module
**Request**
```
POST https://{{VeniceServer}}:{{VenicePort}}/configs/diagnostics/v1/modules/node1-pen-spyglass/Debug
{
	"query": "Log",
	"parameters":{
		"max-results": "300"
	}
}
```
**Response**
```
{
    "diagnostics": {
        "logs": [
            {
                "log": {
                    "@timestamp": "2019-05-10T16:26:07.815Z",
                    "caller": "svc_diagnostics_endpoints.go:488",
                    "duration": "52.865µs",
                    "error": "",
                    "level": "audit",
                    "method": "AutoWatchModule",
                    "module": "pen-spyglass",
                    "msg": "",
                    "pid": "7",
                    "result": "Success",
                    "service": "DiagnosticsV1",
                    "source": "/var/log/pensando/pen-spyglass.log",
                    "ts": "2019-05-10T16:26:07.675716955Z"
                }
            },
            {
                "log": {
                    "@timestamp": "2019-05-10T16:26:10.816Z",
                    "caller": "elastic.go:997",
                    "level": "error",
                    "module": "pen-spyglass",
                    "msg": "request failed, root_cause: [{Type:index_not_found_exception Reason:no such index ResourceType:index_or_alias ResourceId:venice.external.default.configs Index:venice.external.default.configs Phase: Grouped:false CausedBy:map[] RootCause:[] FailedShards:[]}]",
                    "pid": "7",
                    "source": "/var/log/pensando/pen-spyglass.log",
                    "submodule": "elastic-client",
                    "ts": "2019-05-10T16:26:09.85241932Z"
                }
            },
        ]
    }
}
```
### Stats query on module
You can query "cpustats", "memstats", "cmdline" or any expvars that were added. If action is not specified default is get. If expvar is not specified then all expvars are retrieved.
 
**Request**
```
POST https://{{VeniceServer}}:{{VenicePort}}/configs/diagnostics/v1/modules/node1-pen-npm/Debug
     {
     	"query": "Stats",
     	"parameters":{
     		"action": "get",
     		"expvar": "cpustats"
     	}
     }
```
**Response**
```
{
    "diagnostics": {
        "stats": {
            "cpustats": "{\"NumCPUs\":16,\"NumGoroutines\":170,\"NumCgoCalls\":1}"
        }
    }
}
```
### Integration
You can register diagnostics service in your grpc server using following example
```
	fdr.diagSvc = diagsvc.GetDiagnosticsServiceWithDefaults(globals.Spyglass, utils.GetHostname(), diagapi.ModuleStatus_Venice, fdr.rsr, fdr.logger)
	diagnostics.RegisterService(rpcServer.GrpcServer, fdr.diagSvc)
```
You can watch module changes in your grpc service and change log levels like below
```
	// start module watcher when you start your grpc service
	fdr.moduleWatcher = module.GetWatcher(fmt.Sprintf("%s-%s", utils.GetHostname(), globals.Spyglass), globals.APIServer, fdr.rsr, fdr.logger, fdr.moduleChangeCb)

	// define callback to change log level
	func (fdr *Finder) moduleChangeCb(diagmod *diagapi.Module) {
		fdr.logger = log.ResetFilter(diagnostics.GetLogFilter(diagmod.Spec.LogLevel))
		fdr.logger.InfoLog("method", "moduleChangeCb", "msg", "setting log level", "moduleLogLevel", diagmod.Spec.LogLevel)
	}

	// stop module watcher when you stop your service
	fdr.moduleWatcher.Stop()
```