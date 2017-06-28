# Resource management library

Resource library provides a generic mechanism for managing and scheduling resources. This is designed as a library, not a standalone service. That means, its meant to be used by Pensando services to manage resources. Resource library is integrated with kvstore and all resource allocations are persistent to kvstore.

## Concepts

Resource library has four basic concepts:
- Resources: that can be scheduled or allocated
- Providers: Who provides the resource
- Consumers: Who consumes the resource
- Schedulers: Who find the best provider for a consumer

These concepts are loosely based on mesos resource management.

### Resources

**Resource Kinds:** There are three kinds of resources:
  - **Scalar:** Scalar resources are a pool of resource like CPU or memory that can be consumed in any quantity. Scalar resources are represented by a uint64. If you need to schedule fraction of a resource like 1.5 CPU, you can model the resource a milli-cpu and consume 1500 milli-cpus.
  - **Range:** Range resources are descrete ranges like VLAN ids, IP addresses in a subnet or TCAM resources that can be consumed one at a time by a consumer. Once a value in a range is consumed by someone, it can not be used by another consumer. Range resources are represented by uint64 bigining and end values. They are internally stored using a bitmap.
  - **Set:** Set resources are sparse list of values that can be consumed. These are more suitable for managing clusters in a node or disks in a system etc.

**Resource Types:** There could be multiple of types of resources like CPU, memory, IP address, VLAN id etc. Each resource type can have multiple providers. For example a resource of type "Cpu" might be provided by each nodes in the cluster with provider ids "node1", "node2" etc.

**Resource Attributes:** Each resource can have multiple attributes. Each attribute is a key-value pair.

### Providers

Providers provide certain type of resource. For example, a node in a cluster might provide certain amount of memory and CPU resource. A cluster might provide certain amount of VLAN id resources. A subnet might provide IP address resource. Each provider has a unique id called `provider-id`. Provider ID would generally be providing object's key. For example, if a node object provides certain CPU resource, its provider id could be "node1". If a network provides certain IP address resources, its provider could be "network1".

### Consumers

Consumers consume resources by making resource request. Consumers can select the resource providers by specifying a constraints which filter based on the attributes. Consumers can also specify a specific `provider-id` as a constraint to allocate a resource from a specific provider. This is useful for things like allocating IP address from a specific subnet. Allocating TCAM resource on a specific node etc. A resource request can be for any value in a provider or for a specific value. Requesting specific value are useful for cases like allocating specific vlan-ids, allocating specific gateway address within a subnet etc.


### Schedulers

Schedulers are responsible for finding best fit between resource providers and consumers. When a consumer makes a resource request, it can specify the scheduler it wants to use. Currently there is only one scheduler called `LeastUsed`. In future we might add support for more schedulers like `BinPack`, `Spread` etc


## Example

Create a resource manager

```
// create a resource manager instance
rm, err := NewResourceMgr(kvstore)
if err != nil {
    log.Fatalf("Error creating resource manager. Err: %v", err)
}
```

Add a provider

```
// Add a provider
rsrc := &Rsrc{
	RsrcType: "Cpu",
	RsrcKind: ResourceKind_Scalar,
	Scalar: &ScalarRsrc{
		TotalRsrc:     100,
		AvailableRsrc: 100,
	},
}

err := rm.AddProvider(&RsrcProvide{Resource: rsrc, ProviderID: "node1"})
if err != nil {
    log.Errorf("Error creating the provider. Err: %v", err)
    return err
}
```

Request a resource

```
// resource request
req := &RsrcRequest{
	RsrcType:   "Cpu",
	AllocType:  AllocType_Any,
	Scheduler:  "leastUsed",
	Quantity:   4,
	ConsumerID: "container1",
}

// request some resources
consumer, err := rm.RequestResource(req)
if err != nil {
    log.Errorf("Resource request failed with error: %v", err)
    return err
}
```
