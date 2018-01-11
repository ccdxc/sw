package cache

import (
	"testing"
)

func TestPool(t *testing.T) {
	p := connPool{}
	t.Logf("   -> get without an entry")
	x := p.GetFromPool()
	if x != nil {
		t.Fatalf("expecting nil got %+v [ %+v ]", x, &p)
	}

	t.Logf("   -> add an entry")
	p.AddToPool(1)
	x = p.GetFromPool()
	if x.(int) != 1 {
		t.Errorf("expecting added entry got %v", x)
	}
	x = p.GetFromPool()
	if x.(int) != 1 {
		t.Errorf("expecting added entry got %v", x)
	}
	x = p.GetFromPool()
	if x.(int) != 1 {
		t.Errorf("expecting added entry got %v", x)
	}

	t.Logf("   -> add multiple entries in pool")
	entries := []int{1, 2, 3, 4, 5}
	p.AddToPool(2)
	p.AddToPool(3)
	p.AddToPool(4)
	p.AddToPool(5)
	counts := make(map[int]int)
	for _, v := range entries {
		counts[v] = 0
	}

	for i := 0; i < 10; i++ {
		x = p.GetFromPool()
		counts[x.(int)] = counts[x.(int)] + 1
	}
	for _, v := range entries {
		if counts[v] != 2 {
			t.Errorf("unexpected count %d", counts[v])
		}
	}

	t.Logf("   -> delete an entry")
	p.DelFromPool(3)
	if p.total != 4 || len(p.pool) != 4 {
		t.Errorf("Invalid count of entries %d/%d", p.total, len(p.pool))
	}
	for _, v := range entries {
		counts[v] = 0
	}
	for i := 0; i < 10; i++ {
		x = p.GetFromPool()
		counts[x.(int)] = counts[x.(int)] + 1
	}
	if counts[3] != 0 {
		t.Errorf("expecting 0 hits got %d", counts[3])
	}

	t.Logf("   -> delete all entries")
	p.DelAll()
	if len(p.pool) != 0 || p.total != 0 {
		t.Errorf("Found entries when pool should have been empty")
	}
}
