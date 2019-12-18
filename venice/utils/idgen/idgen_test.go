package idgen

import (
	"context"
	"testing"

	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestIDGenerator(t *testing.T) {
	scheme := runtime.NewScheme()

	cfg := Config{
		kvconfig: store.Config{
			Type:    store.KVStoreTypeMemkv,
			Codec:   runtime.NewJSONCodec(scheme),
			Servers: []string{"test-cluster"},
		},
	}
	Init(cfg)

	// Create generator when none exists
	key := "test1"

	ctx := context.Background()
	gen, err := New(ctx, key, SEQUENTIAL)
	AssertOk(t, err, "Creating new generator failed")

	merged := make(map[uint64]bool)
	addID := func(m map[uint64]bool, in uint64) bool {
		_, ok := m[in]
		if ok {
			return false
		}
		m[in] = true
		return true
	}
	id, err := gen.New()
	AssertOk(t, err, "failed to get a new ID")
	Assert(t, addID(merged, id), "got duplicated ID %v", id)

	id, err = gen.New()
	AssertOk(t, err, "failed to get a new ID")
	Assert(t, addID(merged, id), "got duplicated ID %v", id)
	// Create Generator with existing key
	gen2, err := New(ctx, key, SEQUENTIAL)
	AssertOk(t, err, "Creating new generator failed")
	id, err = gen2.New()
	AssertOk(t, err, "failed to get a new ID")
	Assert(t, addID(merged, id), "got duplicated ID %v", id)

	// parallel Id operations
	alloc := func(g Generator, count int, out chan map[uint64]bool) {
		ret := make(map[uint64]bool)
		for j := 0; j < count; j++ {
			i, err := g.New()
			AssertOk(t, err, "ID generation failed (%s)", err)
			Assert(t, addID(ret, i), "got duplicated ID %v", i)
		}
		out <- ret
	}

	ch1 := make(chan map[uint64]bool)
	ch2 := make(chan map[uint64]bool)
	ch3 := make(chan map[uint64]bool)

	go alloc(gen, 100, ch1)
	go alloc(gen2, 100, ch2)
	go alloc(gen, 100, ch3)

	merge := func(in map[uint64]bool) {
		for k := range in {
			Assert(t, addID(merged, k), "got duplicated ID %v", k)
		}
	}
	m1 := <-ch1
	t.Logf("got [%d] ids", len(m1))
	merge(m1)

	m1 = <-ch2
	t.Logf("got [%d] ids", len(m1))
	merge(m1)

	m1 = <-ch3
	t.Logf("got [%d] ids", len(m1))
	merge(m1)
	var max uint64
	for k := range merged {
		if k > max {
			max = k
		}
	}
	t.Logf("Max ID is [%v]", max)

	gen.Close()

	gen3, err := New(ctx, "test2", SEQUENTIAL)
	AssertOk(t, err, "expecting creation to succceed")

	// Destroy original generator
	err = Destroy(ctx, key, SEQUENTIAL)
	AssertOk(t, err, "expecting destroy to succeed")

	_, err = gen2.New()
	Assert(t, err != nil, "generation should fail on destroyed generator")

	_, err = gen3.New()
	AssertOk(t, err, "generation on a different generator should succeed")

	gen2.Close()
	gen3.Close()
	Destroy(ctx, "test2", SEQUENTIAL)
}
