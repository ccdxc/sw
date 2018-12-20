## Palazzo : Venice emulator

Palazzo is an experimental venice emulator that is meant to be used for incremental testing during development.
Its a self contained golang binary that brings up an entire Venice integration test environment (with apigw, apiserver, cmd, controllers, citadel, spyglass, eventsmgr, elastic, agents and nmd) without running the tests. It is not a full fledged Venice cluster like dind cluster or iota cluster. Its an emulator that can be used for testing most of venice code paths during development. Main advantage of palazzo is, it just takes few seconds to compile and bring up venice cluster for development purposes..

Palazzo is fully compatible with Venice UI, Venice CLI, postman recipes etc. Since it runs fully featured apigw, it can even serve Venice UI form its apigw HTTP server. It emulates single Venice node and three Naples nodes. All internal GRPC services and Agent REST api ports are available for testing.

#### Running Palazzo emulator

You can run palazzo on your Mac, your vagrant VM or inside a container. It is a golang process which has no other dependency.

```
$ palazzo
```

This will start the palazzo emulator, within this binary all venice services run as go-routines just like in venice integ environment. If `GOPATH` environment variable is set, palazzo will find the directory where UI files are built and start serving UI files.

After this, you can point your browser to `http://192.168.30.11:9090` (or `http://localhost:9090` if you started it on mac) and you should see the Venice UI. you can point postman to this URL as well and it will work.

you can use Venice CLI to access this cluster too..

```
$ PENSERVER=localhost:9090 venice ls host
name   mac-address        type       
----   -----------        ----       
host0  06:ca:c8:31:89:f3  BAREMETAL  
host1  5e:09:d9:e7:c0:7a  BAREMETAL  
host2  9a:3b:12:6c:f4:f7  BAREMETAL  
```

#### Command line Options

```
$ palazzo --help
 -apiport string
    API Gateway port
 -datapath string
    Specify the datapath type. mock | hal | delphi (default "mock")
 -hosts int
   	Number of hosts / agents (default 3)
 -skipauth
  	Skip authorization in API Gateway
```
