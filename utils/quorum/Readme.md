# quorum

quorum provides a key value(KV) store quorum abstraction for working with various KV stores. The initial implementation only covers etcd.

## Creating a new quorum
```go
import (
       "github.com/pensando/sw/utils/quorum"
       "github.com/pensando/sw/utils/quorum/store"
)

func main() {
...
	members := []quorum.Member {
		{
			Name: "node1",
			PeerURLs: "http://192.168.30.11:5001",
			ClientURLs: "http://192.168.30.11:5002",
		},
		{
			Name: "node2",
			PeerURLs: "http://192.168.30.12:5001",
			ClientURLs: "http://192.168.30.12:5002",
		},
		{
			Name: "node3",
			PeerURLs: "http://192.168.30.13:5001",
			ClientURLs: "http://192.168.30.13:5002",
		},
	}

	config := &quorum.Config{
		Type:       store.KVStoreTypeEtcd,
		ID:         "testCluster",
		DataDir:    "/var/lib/etcd",
		CfgFile:    "/etc/etcd.conf",
		MemberName: "node1",
		Existing:   false,
		Members:    members,
	}

	var err error

	quorumIntf, err := store.New(config)
	if err != nil {
		log.Fatalf("Failed to start quorum with error: %v", err)
	}

	log.Infof("Created quorum successfully")
	...
}
```

## Listing quorum members
```go
	resp, err := quorumIntf.List()
	if err != nil {
		log.Fatalf("Failed to list quorum members with error: %v", err)
	}

	log.Infof("Found members: %v", resp)
```

## Adding a quorum member
```go
	member := &quorum.Member {
			Name: "node4",
			PeerURLs: "http://192.168.30.14:5001",
			ClientURLs: "http://192.168.30.14:5002",
	}
	err := quorumIntf.Add(member)
	if err != nil {
	   log.Fatalf("Failed to add quorum member with error: %v", err)	
	}
	log.Infof("Added member: %v", member)
```

## Removing a quorum member
Use the member id returned from List
```go
	err := quorumIntf.Remove(memberId)
	if err != nil {
	   log.Fatalf("Failed to remove quorum member with error: %v", err)	
	}
	log.Infof("Removed member with id: %v", memberId)
```
