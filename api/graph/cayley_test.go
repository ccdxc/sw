package graph

import (
	"reflect"
	"testing"
)

func TestNewCayleyStore(t *testing.T) {
	r, err := NewCayleyStore()
	if r == nil || err != nil {
		t.Fatalf("Failed to create new Cayley store (%s)", err)
	}
}

func TestDiff(t *testing.T) {
	old := Node{
		Refs: map[string][]string{
			"common.path1": []string{"/obj1", "/obj2", "/obj3"},
			"common.path2": []string{"/obj1", "/obj2"},
			"old.path1":    []string{"/obj/old1", "/obj/old2"},
		},
	}

	new := Node{
		Refs: map[string][]string{
			"common.path1": []string{"/obj1", "/obj2", "/obj3"},
			"common.path2": []string{"/obj1", "/obj3"},
			"new.path1":    []string{"/obj/new1", "/obj/new2"},
		},
	}
	exp := map[string]refDiff{
		"common.path1": refDiff{},
		"common.path2": refDiff{
			add: []string{"/obj3"},
			del: []string{"/obj2"},
		},
		"old.path1": refDiff{
			del: []string{"/obj/old1", "/obj/old2"},
		},
		"new.path1": refDiff{
			add: []string{"/obj/new1", "/obj/new2"},
		},
	}
	c := cayleyStore{}
	d := c.getDiff(&new, &old)
	if !reflect.DeepEqual(d, exp) {
		t.Fatalf("does not match got \n [%+v]\n want \n[%+v\n", d, exp)
	}
}

func TestGetNode(t *testing.T) {
	c, err := NewCayleyStore()
	if err != nil {
		t.Fatalf("could not create store (%s)", err)
	}
	defer c.Close()
	cs := c.(*cayleyStore)
	cs.addRelation("/test/obj11", "/test/obj21", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj22", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj21", "spec.field2", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj23", "spec.field3", labelStrict.String())
	cs.addRelation("/test/obj12", "/test/obj21", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj21", "spec.field2", labelWeak.String())
	cs.addRelation("/test/obj11", "a=b", "spec.Selfield1", labelSelector.String())
	t.Logf("Dump %s", cs.dumpQuads(nil))
	exp := &Node{
		This: "/test/obj11",
		Dir:  RefOut,
		Refs: map[string][]string{
			"spec.field1": {"/test/obj21", "/test/obj22"},
			"spec.field2": {"/test/obj21"},
			"spec.field3": {"/test/obj23"},
		},
		WeakRefs: map[string][]string{
			"spec.field2": {"/test/obj21"},
		},
		SelectorRefs: map[string][]string{
			"spec.Selfield1": {"a=b"},
		},
	}
	n := cs.getNode("/test/obj11", RefOut)
	if n == nil {
		t.Fatalf("could not get node")
	}
	if !reflect.DeepEqual(n, exp) {
		t.Fatalf("Returned node did not match got \n[%+v]\nWant\n[%+v]", n, exp)
	}

	exp1 := &Node{
		This: "/test/obj21",
		Dir:  RefIn,
		Refs: map[string][]string{
			"spec.field1": {"/test/obj11", "/test/obj12"},
			"spec.field2": {"/test/obj11"},
		},
		WeakRefs: map[string][]string{
			"spec.field2": {"/test/obj11"},
		},
		SelectorRefs: make(map[string][]string),
	}
	n1 := cs.getNode("/test/obj21", RefIn)
	if n == nil {
		t.Fatalf("could not get node")
	}
	if !reflect.DeepEqual(n1, exp1) {
		t.Fatalf("Returned node did not match got \n[%+v]\nWant\n[%+v]", n1, exp1)
	}
}

func validateVertex(t *testing.T, in, exp *Vertex) bool {
	if in.This != exp.This {
		t.Logf("did not match got [%s], want [%s]", in.This, exp.This)
		return false
	}
	if in.Dir != exp.Dir {
		t.Logf("Direction did not match got [%v] want [%v]", in.Dir, exp.Dir)
		return false
	}
	if len(in.Refs) != len(exp.Refs) {
		t.Logf("length of refs does not match got\n[%+v]\nwant \n[%+v]", in.Refs, exp.Refs)
		return false
	}
	if len(in.WeakRefs) != len(exp.WeakRefs) {
		t.Logf("length of Weak refs does not match got\n[%+v]\nwant \n[%+v]", in.WeakRefs, exp.WeakRefs)
		return false
	}
	if len(in.SelectorRefs) != len(exp.SelectorRefs) {
		t.Logf("length of Selector refs does not match got\n[%+v]\nwant \n[%+v]", in.SelectorRefs, exp.SelectorRefs)
		return false
	}
	checkRefs := func(id string, inf, expf map[string][]*Vertex) bool {
		for k, v := range inf {
			v1, ok := expf[k]
			if !ok {
				t.Fatalf("for [%v] did not find [%v] in Refs", id, k)
				return false
			}
			if len(v) != len(v1) {
				t.Logf("number of references in [%v] did not match got\n[%+v]\n want\n[%+v]", k, v, v1)
				return false
			}
			keys := make(map[string]*Vertex)
			for i := range v1 {
				if _, ok := keys[v1[i].This]; ok {
					t.Logf("Duplicate Vertex [%v]", v1[i])
					return false
				}
				keys[v1[i].This] = v1[i]
			}
			for _, v2 := range inf[k] {
				if v2 == nil {
					t.Fatalf("got nil for [%+v][%v]", id, k)
				}
				e2, ok := keys[v2.This]
				if !ok {
					t.Logf("failed to find vertex for [%v] in [%+v]", v2.This, keys)
					return false
				}
				if !validateVertex(t, v2, e2) {
					t.Logf("failed to validate [%v]", v2.This)
					return false
				}
			}
		}
		return true
	}
	if !checkRefs(in.This, in.Refs, exp.Refs) {
		t.Logf("Strict refs failed[%v]", in.This)
		return false
	}
	if !checkRefs(in.This, in.WeakRefs, exp.WeakRefs) {
		t.Logf("WeakRefs refs failed[%v]", in.This)
		return false
	}
	if len(in.SelectorRefs) > 0 {
		t.Logf("Selector refs failed[%v]", in.This)
		if !reflect.DeepEqual(in.SelectorRefs, exp.SelectorRefs) {
			return false
		}
	}

	return true
}

func TestGetVertex(t *testing.T) {
	c, err := NewCayleyStore()
	if err != nil {
		t.Fatalf("could not create store (%s)", err)
	}
	defer c.Close()
	cs := c.(*cayleyStore)
	cs.addRelation("/test/obj11", "/test/obj21", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj22", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj23", "spec.field2", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj24", "spec.field2", labelStrict.String())
	cs.addRelation("/test/obj21", "/test/obj31", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj22", "/test/obj32", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj22", "/test/obj33", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj23", "/test/obj34", "spec.field3", labelStrict.String())
	cs.addRelation("/test/obj23", "/test/obj35", "spec.field3", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj25", "spec.field3", labelWeak.String())
	cs.addRelation("/test/obj11", "ab=cd", "spec.selfield1", labelSelector.String())

	visited := make(map[string]*Vertex)
	exp := &Vertex{
		This: "/test/obj11",
		Dir:  RefOut,
		Refs: map[string][]*Vertex{
			"spec.field1": {
				&Vertex{
					This: "/test/obj21",
					Dir:  RefOut,
					Refs: map[string][]*Vertex{
						"spec.field1": {
							&Vertex{
								This: "/test/obj31",
								Dir:  RefOut,
							},
						},
					},
				},
				&Vertex{
					This: "/test/obj22",
					Dir:  RefOut,
					Refs: map[string][]*Vertex{
						"spec.field1": {
							&Vertex{
								This: "/test/obj32",
								Dir:  RefOut,
							},
							&Vertex{
								This: "/test/obj33",
								Dir:  RefOut,
							},
						},
					},
				},
			},
			"spec.field2": {
				&Vertex{
					This: "/test/obj23",
					Dir:  RefOut,
					Refs: map[string][]*Vertex{
						"spec.field3": {
							&Vertex{
								This: "/test/obj34",
								Dir:  RefOut,
							},
							&Vertex{
								This: "/test/obj35",
								Dir:  RefOut,
							},
						},
					},
				},
				&Vertex{
					This: "/test/obj24",
					Dir:  RefOut,
				},
			},
		},
		WeakRefs: map[string][]*Vertex{
			"spec.field3": {
				&Vertex{
					This: "/test/obj25",
					Dir:  RefOut,
					Refs: make(map[string][]*Vertex),
				},
			},
		},
		SelectorRefs: map[string][]string{
			"spec.selfield1": {"ab=cd"},
		},
	}
	v := cs.getVertex("/test/obj11", RefOut, visited)
	if v == nil {
		t.Fatalf("failed to get Vertex")
	}
	if !validateVertex(t, v, exp) {
		t.Fatalf("the vertex does not match expectation\n got\n[%+v]\nwant\n[%+v]", v, exp)
	}

	visited = make(map[string]*Vertex)
	exp1 := &Vertex{
		This: "/test/obj33",
		Dir:  RefIn,
		Refs: map[string][]*Vertex{
			"spec.field1": {
				&Vertex{
					This: "/test/obj22",
					Dir:  RefIn,
					Refs: map[string][]*Vertex{
						"spec.field1": {
							&Vertex{
								This: "/test/obj11",
								Dir:  RefIn,
							},
						},
					},
				},
			},
		},
	}
	v = cs.getVertex("/test/obj33", RefIn, visited)
	if v == nil {
		t.Fatalf("failed to get Vertex")
	}
	if !validateVertex(t, v, exp1) {
		t.Fatalf("the vertex does not match expectation\n got\n[%+v]\nwant[%+v]", v, exp1)
	}
}

func TestAddDelRelation(t *testing.T) {
	c, err := NewCayleyStore()
	if err != nil {
		t.Fatalf("could not create store (%s)", err)
	}
	defer c.Close()
	cs := c.(*cayleyStore)
	cs.addRelation("/test/obj11", "/test/obj21", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj22", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj21", "spec.field2", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj23", "spec.field3", labelStrict.String())
	cs.addRelation("/test/obj12", "/test/obj21", "spec.field1", labelStrict.String())
	t.Logf("Dump %s", cs.dumpQuads(nil))
	exp := &Node{
		This: "/test/obj11",
		Dir:  RefOut,
		Refs: map[string][]string{
			"spec.field1": {"/test/obj21", "/test/obj22"},
			"spec.field2": {"/test/obj21"},
			"spec.field3": {"/test/obj23"},
		},
		WeakRefs:     make(map[string][]string),
		SelectorRefs: make(map[string][]string),
	}
	n := cs.getNode("/test/obj11", RefOut)
	if n == nil {
		t.Fatalf("could not get node")
	}
	if !reflect.DeepEqual(n, exp) {
		t.Fatalf("Returned node did not match got \n[%+v]\nWant\n[%+v]", n, exp)
	}

	cs.delRelation("/test/obj11", "/test/obj22", "spec.field1", labelStrict.String())
	exp1 := &Node{
		This: "/test/obj11",
		Dir:  RefOut,
		Refs: map[string][]string{
			"spec.field1": {"/test/obj21"},
			"spec.field2": {"/test/obj21"},
			"spec.field3": {"/test/obj23"},
		},
		WeakRefs:     make(map[string][]string),
		SelectorRefs: make(map[string][]string),
	}
	n = cs.getNode("/test/obj11", RefOut)
	if n == nil {
		t.Fatalf("could not get node")
	}
	if !reflect.DeepEqual(n, exp1) {
		t.Fatalf("Returned node did not match got \n[%+v]\nWant\n[%+v]", n, exp1)
	}
}

func TestUpdateDeleteNode(t *testing.T) {
	c, err := NewCayleyStore()
	if err != nil {
		t.Fatalf("could not create store (%s)", err)
	}
	defer c.Close()
	cs := c.(*cayleyStore)
	cs.addRelation("/test/obj11", "/test/obj21", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj22", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj21", "spec.field2", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj23", "spec.field3", labelStrict.String())
	cs.addRelation("/test/obj12", "/test/obj21", "spec.field1", labelStrict.String())
	t.Logf("Dump %s", cs.dumpQuads(nil))
	exp := &Node{
		This: "/test/obj11",
		Dir:  RefOut,
		Refs: map[string][]string{
			"spec.field1": {"/test/obj21", "/test/obj22"},
			"spec.field2": {"/test/obj21"},
			"spec.field3": {"/test/obj23"},
		},
		WeakRefs:     make(map[string][]string),
		SelectorRefs: make(map[string][]string),
	}
	n := cs.getNode("/test/obj11", RefOut)
	if n == nil {
		t.Fatalf("could not get node")
	}
	if !reflect.DeepEqual(n, exp) {
		t.Fatalf("Returned node did not match got \n[%+v]\nWant\n[%+v]", n, exp)
	}
	cs.DeleteNode("/test/obj22")
	t.Logf("Dump %s", cs.dumpQuads(nil))
	n = cs.getNode("/test/obj22", RefIn)
	if n != nil {
		t.Fatalf("found node that was deleted")
	}
	n = cs.getNode("/test/obj22", RefOut)
	if n != nil {
		t.Fatalf("found node that was deleted")
	}

	exp1 := &Node{
		This: "/test/obj11",
		Dir:  RefOut,
		Refs: map[string][]string{
			"spec.field1": {"/test/obj21"},
			"spec.field2": {"/test/obj21"},
			"spec.field3": {"/test/obj23"},
		},
		WeakRefs:     make(map[string][]string),
		SelectorRefs: make(map[string][]string),
	}
	n = cs.getNode("/test/obj11", RefOut)
	if n == nil {
		t.Fatalf("could not get node")
	}
	if !reflect.DeepEqual(n, exp1) {
		t.Fatalf("Returned node did not match got \n[%+v]\nWant\n[%+v]", n, exp1)
	}
}

func TestIsolated(t *testing.T) {
	c, err := NewCayleyStore()
	if err != nil {
		t.Fatalf("could not create store (%s)", err)
	}

	defer c.Close()
	cs := c.(*cayleyStore)

	cs.addRelation("/test/obj11", "/test/obj21", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj22", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj21", "spec.field2", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj23", "spec.field3", labelWeak.String())
	cs.addRelation("/test/obj12", "/test/obj21", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj12", "/test/obj22", "spec.field1", labelWeak.String())
	t.Logf("Dump %s", cs.dumpQuads(nil))
	exp := map[string]bool{
		"/test/obj11": true,
		"/test/obj21": false,
		"/test/obj22": false,
		"/test/obj23": true,
		"/test/obj12": true,
		"/test/dummy": true,
	}
	for k := range exp {
		if exp[k] != c.IsIsolated(k) {
			t.Fatalf("[%v] did not match expn [%v] got [%v]", k, c.IsIsolated(k), exp[k])
		}
	}
}

func TestUpdateNode(t *testing.T) {
	c, err := NewCayleyStore()
	if err != nil {
		t.Fatalf("could not create store (%s)", err)
	}
	defer c.Close()
	cs := c.(*cayleyStore)
	cs.addRelation("/test/obj11", "/test/obj21", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj22", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj21", "spec.field2", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj23", "spec.field3", labelStrict.String())
	cs.addRelation("/test/obj12", "/test/obj21", "spec.field1", labelStrict.String())
	cs.addRelation("/test/obj11", "/test/obj21", "spec.wfield1", labelWeak.String())
	cs.addRelation("/test/obj11", "a=c", "spec.selfield1", labelSelector.String())
	t.Logf("Dump %s", cs.dumpQuads(nil))
	exp := &Node{
		This: "/test/obj11",
		Dir:  RefOut,
		Refs: map[string][]string{
			"spec.field1": {"/test/obj21", "/test/obj22"},
			"spec.field2": {"/test/obj21"},
			"spec.field3": {"/test/obj23"},
		},
		WeakRefs: map[string][]string{
			"spec.wfield1": {"/test/obj21"},
		},
		SelectorRefs: map[string][]string{
			"spec.selfield1": {"a=c"},
		},
	}
	n := cs.getNode("/test/obj11", RefOut)
	if n == nil {
		t.Fatalf("could not get node")
	}
	if !reflect.DeepEqual(n, exp) {
		t.Fatalf("Returned node did not match got \n[%+v]\nWant\n[%+v]", n, exp)
	}
	// Add an entry and delete an entry
	n.Refs["spec.field1"] = []string{"/test/obj21"}
	n.Refs["spec.field3"] = []string{"/test/obj23"}
	n.WeakRefs["spec.field4"] = []string{"/test/obj23"}
	n.SelectorRefs["spec.selfield1"] = []string{"a=b"}
	exp = n
	err = c.UpdateNode(n)
	if err != nil {
		t.Fatalf("failed to update node (%s)", err)
	}

	n = cs.getNode("/test/obj11", RefOut)
	if n == nil {
		t.Fatalf("could not get node")
	}
	if !reflect.DeepEqual(n, exp) {
		t.Fatalf("Returned node did not match got \n[%+v]\nWant\n[%+v]", n, exp)
	}
}
