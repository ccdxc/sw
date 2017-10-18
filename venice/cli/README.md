
### How to add  new cli command

#### 1. Add an entry in cli registry
CLI registry can be found in `venice/cli/api/types.go`. Modify following structure to add/modify
an entry for your proto definition objects.
```
    var Objs = map[string]ObjectInfo{
	"node": {Name: "node", Package: "cmd", GrpcService: "cmd", URL: "/v1/cmd/nodes", Perms: "rw",
		Structs: []string{"NodeCondition", "PortCondition", "ConditionStatus"}},
```

### 2. Any new pkgs in the pkg list
pkg list can be found in `venice/cli/api/pkgs.txt` directory. Add any new package there

### 3. Precompile your changes
CLI generation is not run automatically (or as a dependency change), rather must be manually triggered
using the following top level make target.
```
make pregen
```

### 4. Add unit test cases for your cli
Unit test cases are found in `venice/cli/cli_test.go`. Without this, coverage test cases will fail,
and you will not be able to verify if anyone else is breaking your module's CLI.

### 5. Run unit tests, integration tests and sanity runs as usual
