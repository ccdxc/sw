# Pensando API
- ./protos - all API definitions as protobuf definitions.
- ./generated - all generated code based on the API definitions in /protos
- ./hooks - handwritten plugins for the api server.

# Guidelines for writing a proto definition
Proto definitions **MUST** follow the following guidelines in order to have a consistent
infrastructure and user experience within and oustide the system. Details and explanations
can be found [here](https://docs.google.com/document/d/1gjRW_snyaWi313A_Nuw_lA351LU71jnZZ5Rrl2jk0QE)
This document is **NOT** a place to learn the structure of code generation infrastructure,
which can be found [here](../docs/apidef.md)

### 1. TypeMeta **MUST** be inline, and TypeMeta and ObjectMeta be named specifically

This is done by specifying `(gogoproto.nullable) = false` and `(gogorpoto.jsontag) = ",inline"` tags in the definition.
e.g. in the definition below `api.TypeMeta` and `api.ObjectMeta` are defined like that. Also note that their names **MUST**
always be `T` and `O` for consistency.

```
message Cluster {

    option (venice.objectPrefix) = "cluster";

    api.TypeMeta T      = 1 [(gogoproto.embed) = true,
                            (gogoproto.nullable) = false,
                            (gogoproto.jsontag) = ",inline"];

    api.ObjectMeta O    = 2 [(gogoproto.embed) = true,
                            (gogoproto.nullable) = false,
                            (gogoproto.jsontag) = "meta,omitempty"];

    ClusterSpec Spec     = 3 [(gogoproto.nullable) = false,
                             (gogoproto.jsontag) = "spec,omitempty"];

    ClusterStatus Status = 4 [(gogoproto.nullable) = false,
                             (gogoproto.jsontag) = "status,omitempty"];
}
```

### 2. Top level structs **MUST NOT** be nullable
Top level structs, i.e. `TypeMeta`, `ObjectMeta`, `Spec` and `Status` **MUST NOT** be nullable. This would result generated code to not have pointers.
e.g. definitions below use `(gogoproto.nullable) = false` to define it like this

```
message Cluster {

    option (venice.objectPrefix) = "cluster";

    api.TypeMeta T      = 1 [(gogoproto.embed) = true,
                            (gogoproto.nullable) = false,
                            (gogoproto.jsontag) = ",inline"];

    api.ObjectMeta O    = 2 [(gogoproto.embed) = true,
                            (gogoproto.nullable) = false,
                            (gogoproto.jsontag) = "meta,omitempty"];

    ClusterSpec Spec     = 3 [(gogoproto.nullable) = false,
                             (gogoproto.jsontag) = "spec,omitempty"];

    ClusterStatus Status = 4 [(gogoproto.nullable) = false,
                             (gogoproto.jsontag) = "status,omitempty"];
}
```

### 3. Field Names and Json Tags must be camelCased and have a description
All field names within a proto definition **MUST** be CamelCased and first letter **MUST** be capital for struct definitions.
Json tags must use first letter as lowerCase and the name *MAY* differ from the field name, likely matching user expectation about a particular field.
e.g. various fields names, json tags, and field description in the following definition
```go
message ExportPolicySpec {
    // Export Interval defines how often to push the records to an external or internal collector
    // The value is specified as a string format to be '10s', '20m', '20mins', '10secs', '10seconds'
    string ExportInterval = 1 [(gogoproto.jsontag) = "exportInterval,omitempty"];

    // IP address of the collector/entity to which the data is to be exported
    // When unspecified venice automatically becomes the data collector
    string CollectorIpAddress = 2 [(gogoproto.jsontag) = "collectorIpAddress,omitempty"];

    // Protocol and Port number where an external collector is gathering the data
    string CollectorPort = 3 [(gogoproto.jsontag) = "collectorPort,omitempty"];

    // Format is an enumeration from the list "netflow", "collectd", "kafka", etc. (TBD: choices)
    string Format = 4 [(gogoproto.jsontag) = "format,omitempty"];
}
```
When putting acronyms within a camelCased field, capitalize only first letter e.g.
Instead of `serverIPAddress` use `serverIpAddress`, or instead of `DNSServerName` use `DnsServerName` or `dnsServerName`

### 4. List Objects
List objects are automatically generated, therefore they **MUST NOT** be declared explicitly
e.g. if we declare following definition
```
// Endpoint represents a network endpoint
message Endpoint {
    option (venice.objectPrefix) = "endpoint";
    api.TypeMeta T =1 [(gogoproto.embed) = true, (gogoproto.nullable) = false, (gogoproto.jsontag) = ",inline"];
    api.ObjectMeta O =2 [(gogoproto.embed) = true, (gogoproto.nullable) = false, (gogoproto.jsontag) = "meta,omitempty"];

    // Spec contains the configuration of the Endpoint.
    EndpointSpec Spec = 3 [(gogoproto.nullable) = false, (gogoproto.jsontag) = "spec,omitempty"];

    // Status contains the current state of the Endpoint.
    EndpointStatus Status = 4 [(gogoproto.nullable) = false, (gogoproto.jsontag) = "status,omitempty"];
}
```
Proto generation code would generate the list object automatically, called `EndpointList`
```
// Endpoint represents a network endpoint
message Endpoint {
    option (venice.objectPrefix) = "endpoint";
    api.TypeMeta T =1 [(gogoproto.embed) = true, (gogoproto.nullable) = false, (gogoproto.jsontag) = ",inline"];
    api.ObjectMeta O =2 [(gogoproto.embed) = true, (gogoproto.nullable) = false, (gogoproto.jsontag) = "meta,omitempty"];

    // Spec contains the configuration of the Endpoint.
    EndpointSpec Spec = 3 [(gogoproto.nullable) = false, (gogoproto.jsontag) = "spec,omitempty"];

    // Status contains the current state of the Endpoint.
    EndpointStatus Status = 4 [(gogoproto.nullable) = false, (gogoproto.jsontag) = "status,omitempty"];
}

// EndpointList represents a list of network endpoints
type EndpointList struct {
        api.TypeMeta `protobuf:"bytes,2,opt,name=T,embedded=T" json:"T"`
        api.ListMeta `protobuf:"bytes,3,opt,name=ListMeta,embedded=ListMeta" json:"ListMeta"`

        // Spec contains the configuration of the security group.
        Items []*Endpoint `protobuf:"bytes,4,rep,name=Items" json:"Items,omitempty"`
}
```

### 5. Venice Tags
Venice tags in proto definitions allow for special behavior on those fields. They *SHOULD* be used
whenever appropriate

. `venice.check`: used to specify the validation functions on various fields, e.g.
```
    string WebAddr = 4 [(venice.check) = "URI()", (venice.check) = "StrLen(6,256)"];
```

. `venice.cli`: used to specify the attributes that defines the CLI behavior
More documentation TBD

. String Enums using `venice.check`: Often string fields can only have a few enumerated values.
String enums are defined as enums with `venice.StrEnum` tag e.g. in the following example a
`NetworkType` field is permitted to have specific values
```
message Network {
  enum  NetworkTypes{
    Vlan = 0;
    Vxlan = 1;
  }
  string NetworkType = 5 [(venice.check) = "StrEnum(NetworkSpec.NetworkTypes)"];
}
```

### 6. Plural fields **SHOULD** be appended with an `s`
```
repeated SGRule InRules      = 2 [(gogoproto.nullable) = false, (gogoproto.jsontag) = "inRules,omitempty"];
```
