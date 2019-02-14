package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"math/rand"
	"strconv"
	"sync"
	"time"

	"github.com/cayleygraph/cayley"
	"github.com/cayleygraph/cayley/quad"

	hdr "github.com/pensando/sw/venice/utils/histogram"
)

func doquery(count, maxCount int, name string, store *cayley.Handle, wg *sync.WaitGroup) {
	r := rand.New(rand.NewSource(int64(time.Now().Nanosecond())))
	for i := 0; i < count; i++ {
		start := time.Now()
		cayley.StartPath(store, quad.String(fmt.Sprintf("rule-%d", r.Intn(maxCount)))).Out(quad.String("used-by")).Iterate(nil).EachValue(nil, func(value quad.Value) {
			// fmt.Printf("value is %v\n", quad.NativeOf(value))
		})
		hdr.Record(name, time.Since(start))
	}
	wg.Done()
}

func benchM() {
	store, err := cayley.NewMemoryGraph()
	if err != nil {
		log.Fatalln(err)
	}
	// insert N x 2 quads
	start := time.Now()
	for i := 0; i < 700000; i++ {
		id := fmt.Sprintf("%d", i)
		for k := 0; k < len(id); k++ {
			v, err := strconv.ParseInt(string(id[k]), 10, 32)
			if err != nil {
				fmt.Printf("failed to convert to val %v", id[k])
			}
			store.AddQuad(quad.Make(fmt.Sprintf("rule-%d", i), "used-by", fmt.Sprintf("policy-%d", v), nil))
			store.AddQuad(quad.Make(fmt.Sprintf("policy-%d", v), "uses", fmt.Sprintf("rule-%d", i), nil))
		}
	}
	fmt.Printf("creating quads took %s\n", time.Since(start))

	// Start 100 go routines quering a 1000 entries each
	start = time.Now()
	var wg sync.WaitGroup
	for i := 0; i < 3; i++ {
		wg.Add(1)
		go doquery(100000, 700000, "Query", store, &wg)
	}
	wg.Wait()
	fmt.Printf("Completed queries in %s \n", time.Since(start))
	hdr.PrintAll()
}

func main() {
	benchmark := flag.Bool("b", false, "run perf benchmark")

	store, err := cayley.NewMemoryGraph()
	if err != nil {
		log.Fatalln(err)
	}

	if *benchmark {
		benchM()
	}

	rels := []struct {
		sub, pred, obj, label string
	}{
		{"alice", "follows", "bob", ""},
		{"charlie", "follows", "bob", ""},
		{"dani", "follows", "bob", ""},
		{"bob", "follows", "fred", ""},
		{"fred", "follows", "greg", ""},
		{"emily", "follows", "fred", ""},
		{"dani", "follows", "greg", ""},
		{"dani", "status", "cool_person", ""},
		{"bob", "status", "cool_person", ""},
		{"greg", "status", "cool_person", ""},
		{"greg", "status", "smart_person", "smart_graph"},
		{"emily", "status", "smart_person", "smart_graph"},
	}

	for _, v := range rels {
		var labels interface{}
		if v.label == "" {
			labels = nil
		} else {
			labels = v.label
		}
		store.AddQuad(quad.Make(v.sub, v.pred, v.obj, labels))
	}
	fmt.Println(store.Size())
	{

		it := store.QuadsAllIterator()
		for it.Next(context.TODO()) {
			fmt.Printf("=> [%+v]\n", store.Quad(it.Result()))
		}
		m := cayley.StartMorphism().In("follows")
		p := cayley.StartPath(store, quad.String("bob")).FollowRecursive(m, -1, nil)
		preds := p.OutPredicates()
		fmt.Printf("Predicates are [%v]\n", preds)
		it2, err := preds.Iterate(context.TODO()).AllValues(store)
		if err != nil {
			fmt.Printf("could not initialize iterator (%s)", err)
		}
		test := "Example"
		for i := range it2 {
			fmt.Printf("==>[%s] %s\n", test, it2[i])
		}
		p1, _ := cayley.StartPath(store, quad.String("bob")).In(nil).BuildIteratorOn(store).Optimize()
		p1, _ = store.OptimizeIterator(p1)
		for p1.Next(context.TODO()) {
			res := store.Quad(p1.Result())
			fmt.Printf("Token [%v] [%v] [%v] via [%v] to [%v]", p1.Result(), store.NameOf(p1.Result()), res.Subject, res.Predicate, res.Object)
		}
		it1, err := p.Iterate(context.TODO()).AllValues(store)
		if err != nil {
			fmt.Printf("could not initialize iterator (%s)", err)
		}
		for i := range it1 {
			fmt.Printf("==> [%v]\n", it1[i])
		}
	}

	{
		ctx := context.TODO()
		it := store.QuadIterator(quad.Object, store.ValueOf(quad.String("bob")))
		for it.Next(ctx) {
			q := store.Quad(it.Result())
			fmt.Printf("---> [%v] [%v] [%v]\n", q.Subject, q.Predicate, q.Object)
		}
	}

}
