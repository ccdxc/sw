package memkv

import (
	"fmt"
	"math/rand"
	"sync"
	"testing"
	"github.com/pensando/sw/utils/kvstore"
	"github.com/pensando/sw/utils/runtime"
)

type memkvElection struct {
	leader		string
	termId		int
	contenders	[]*election
}

type memkvCluster struct {
	sync.Mutex
	elections	map[string]*memkvElection	// current elections
	clientId 	int				// current id of the store
	stores		map[string]*memKv		// all client stores
}

func clusterSetup(t *testing.T) (kvstore.TestCluster) {
	rand.Seed(74)
	return &memkvCluster{
		elections:	make(map[string]*memkvElection),
		clientId: 	0,
		stores : 	make(map[string]*memKv),
	}
}

func clusterCleanup(t *testing.T, cluster kvstore.TestCluster) {
	c, ok := cluster.(*memkvCluster)
	if !ok {
		t.Fatalf("invalid cluster")
	}

	c.Lock()
	defer c.Unlock()

	// walk all state stores and delete keys
	for memkv_key, f := range c.stores {
		f.deleteAll()
		delete (c.stores, memkv_key)
	}
}

func storeSetup(t *testing.T, cluster kvstore.TestCluster) (kvstore.Interface, error) {
	c, ok := cluster.(*memkvCluster)
	if !ok {
		t.Fatalf("invalid cluster")
	}

	c.Lock()
	defer c.Unlock()

	c.clientId += 1
	clientName := fmt.Sprintf("memkv-%d", c.clientId)

	s := runtime.NewScheme()
	s.AddKnownTypes(&kvstore.TestObj{}, &kvstore.TestObjList{})
	store, _ := NewMemKv(c, runtime.NewJSONCodec(s))
	cs, ok := store.(*memKv);
	if !ok {
		t.Fatalf("invalid store")
	}
	c.stores[clientName] = cs

	return store, nil
}

func TestRunAll(t *testing.T) {
	kvstore.RunInterfaceTests(t, clusterSetup, storeSetup, clusterCleanup)
}
