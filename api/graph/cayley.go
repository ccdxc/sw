package graph

import (
	"context"
	"fmt"
	"sync"

	"github.com/cayleygraph/cayley"
	"github.com/cayleygraph/cayley/graph"
	"github.com/cayleygraph/cayley/graph/shape"
	"github.com/cayleygraph/cayley/quad"
	"github.com/cayleygraph/cayley/schema"
	"github.com/deckarep/golang-set"
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/log"
)

type cayleyStore struct {
	ctx    context.Context
	cancel context.CancelFunc
	sync.Mutex
	store  cayley.QuadStore
	sch    *schema.Config
	writer graph.QuadWriter
}

var once sync.Once

// The graph DB is comprised of a set of Quads. Each Quad is a tuple of
//  (subject, predicate, object, label) - Where the subject can be viewed as the
//  source node of the graph, the predicate is the edge and the object is the destination
//  node. A label is the context of the link. The label is used to construct multiple graphs or
//  multiple "named" graphs. In our implementation, the label is used to construct multiple graphs
//  in the DB, one each for a type of reference - strict, weak and selector reference.
const (
	labelStrict   relLabel = "strictRef"
	labelWeak     relLabel = "weakRef"
	labelSelector relLabel = "selectorRef"
)

type relLabel string

// String returns the string for the label
func (in relLabel) String() string {
	return string(in)
}

// NewCayleyStore returns a graph.Interface with a cayley store backend.
func NewCayleyStore() (Interface, error) {
	ret := &cayleyStore{}
	var err error
	store, err := cayley.NewMemoryGraph()
	if err != nil {
		log.ErrorLog("msg", "creating cayley store failed (%s)", err)
		return nil, err
	}
	// ret.writer = graph.NewWriter(ret.store)
	ret.store = store.QuadStore
	ret.writer = store.QuadWriter
	ret.ctx, ret.cancel = context.WithCancel(context.Background())
	return ret, nil
}

type refDiff struct {
	add  []string
	del  []string
	wadd []string
	wdel []string
	sadd string
	sdel string
}

func (s *cayleyStore) refDiff(old, new []string) (a, d []string) {
	var nset, oset []interface{}
	for i := range new {
		nset = append(nset, new[i])
	}
	for i := range old {
		oset = append(oset, old[i])
	}
	ns := mapset.NewSetFromSlice(nset)
	os := mapset.NewSetFromSlice(oset)
	add := ns.Difference(os).ToSlice()
	del := os.Difference(ns).ToSlice()

	for i := range add {
		a = append(a, add[i].(string))
	}
	for i := range del {
		d = append(d, del[i].(string))
	}
	return
}

func (s *cayleyStore) getDiff(new, old *Node) map[string]refDiff {
	// XXX-TODO(sanjayt): move to transaction
	ret := make(map[string]refDiff)
	if old == nil {
		old = &Node{}
	}
	for k, v := range new.Refs {
		rd, ok := ret[k]
		if !ok {
			rd = refDiff{}
		}
		if o, ok := old.Refs[k]; ok {
			add, del := s.refDiff(o, v)
			rd.add = append(rd.add, add...)
			rd.del = append(rd.del, del...)
		} else {
			rd.add = v
		}
		ret[k] = rd
	}
	for k, v := range old.Refs {
		if _, ok := new.Refs[k]; !ok {
			rd, ok := ret[k]
			if !ok {
				rd = refDiff{}
			}
			rd.del = v
			ret[k] = rd
		}
	}
	for k, v := range new.WeakRefs {
		rd, ok := ret[k]
		if !ok {
			rd = refDiff{}
		}
		if o, ok := old.WeakRefs[k]; ok {
			add, del := s.refDiff(o, v)
			rd.wadd = append(rd.wadd, add...)
			rd.wdel = append(rd.wdel, del...)
		} else {
			rd.wadd = v
		}
		ret[k] = rd
	}
	for k, v := range old.WeakRefs {
		if _, ok := new.WeakRefs[k]; !ok {
			rd, ok := ret[k]
			if !ok {
				rd = refDiff{}
			}
			rd.wdel = v
			ret[k] = rd
		}
	}
	for k, v := range old.SelectorRefs {
		// We can assume there will be only one entry in the Refs
		news := ""
		if v1, ok := new.SelectorRefs[k]; ok {
			news = v1[0]
		}
		if v[0] != news {
			rd, ok := ret[k]
			if !ok {
				rd = refDiff{}
			}
			rd.sdel = v[0]
			rd.sadd = news
			ret[k] = rd
		}
	}
	for k, v := range new.SelectorRefs {
		if _, ok := old.SelectorRefs[k]; !ok {
			rd, ok := ret[k]
			if !ok {
				rd = refDiff{}
			}
			rd.sadd = v[0]
			ret[k] = rd
		}
	}
	return ret
}

func (s *cayleyStore) getNode(id string, dir Direction) *Node {
	var ret Node
	var d quad.Direction
	if dir == RefOut {
		d = quad.Subject
	} else {
		d = quad.Object
	}
	ret.Dir = dir
	ret.This = id
	ret.Refs = make(map[string][]string)
	ret.WeakRefs = make(map[string][]string)
	ret.SelectorRefs = make(map[string][]string)
	it := s.store.QuadIterator(d, s.store.ValueOf(quad.String(id)))
	if i, _ := it.Size(); i == 0 {
		return nil
	}
	defer it.Close()
	for it.Next(s.ctx) {
		q := s.store.Quad(it.Result())
		other := q.Object
		if dir == RefIn {
			other = q.Subject
		}
		label := q.Label.Native().(string)
		switch label {
		case labelStrict.String():
			ret.Refs[q.Predicate.Native().(string)] = append(ret.Refs[q.Predicate.Native().(string)], other.Native().(string))
		case labelWeak.String():
			ret.WeakRefs[q.Predicate.Native().(string)] = append(ret.WeakRefs[q.Predicate.Native().(string)], other.Native().(string))
		case labelSelector.String():
			ret.SelectorRefs[q.Predicate.Native().(string)] = append(ret.SelectorRefs[q.Predicate.Native().(string)], other.Native().(string))
		}
	}
	return &ret
}

func (s *cayleyStore) getVertex(id string, dir Direction, visited map[string]*Vertex) *Vertex {
	if v, ok := visited[id]; ok {
		return v
	}
	var ret Vertex
	var d quad.Direction
	if dir == RefOut {
		d = quad.Subject
	} else {
		d = quad.Object
	}
	ret.Dir = dir
	ret.This = id
	ret.Refs = make(map[string][]*Vertex)
	ret.WeakRefs = make(map[string][]*Vertex)
	ret.SelectorRefs = make(map[string][]string)
	visited[id] = &ret
	it := s.store.QuadIterator(d, s.store.ValueOf(quad.String(id)))
	defer it.Close()
	for it.Next(s.ctx) {
		q := s.store.Quad(it.Result())
		other := q.Object
		if dir == RefIn {
			other = q.Subject
		}
		label := q.Label.Native().(string)
		switch label {
		case labelStrict.String():
			ret.Refs[q.Predicate.Native().(string)] = append(ret.Refs[q.Predicate.Native().(string)], s.getVertex(other.Native().(string), dir, visited))
		case labelWeak.String():
			ret.WeakRefs[q.Predicate.Native().(string)] = append(ret.WeakRefs[q.Predicate.Native().(string)], s.getVertex(other.Native().(string), dir, visited))
		case labelSelector.String():
			ret.SelectorRefs[q.Predicate.Native().(string)] = append(ret.SelectorRefs[q.Predicate.Native().(string)], other.Native().(string))
		}
	}
	return &ret
}

func (s *cayleyStore) addRelation(sub, obj, pred, label string) error {
	return s.writer.AddQuad(quad.Make(sub, pred, quad.String(obj), quad.String(label)))
}

func (s *cayleyStore) delRelation(sub, obj, pred, label string) error {
	return s.writer.RemoveQuad(quad.Make(quad.String(sub), quad.String(pred), quad.String(obj), quad.String(label)))
}

// UpdateNode updates the node with proper references. It takes care of taking a diff wrt current node
//   and applying the diff to the node.
func (s *cayleyStore) UpdateNode(in *Node) error {
	if err := s.ctx.Err(); err != nil {
		return err
	}
	defer s.Unlock()
	s.Lock()
	// create update and delete sets
	old := s.getNode(in.This, RefOut)
	odiff := s.getDiff(in, old)
	for pred, d := range odiff {
		for i := range d.add {
			s.addRelation(in.This, d.add[i], pred, labelStrict.String())
		}
		for i := range d.del {
			s.delRelation(in.This, d.del[i], pred, labelStrict.String())
		}
		for i := range d.wadd {
			s.addRelation(in.This, d.wadd[i], pred, labelWeak.String())
		}
		for i := range d.wdel {
			s.delRelation(in.This, d.wdel[i], pred, labelWeak.String())
		}
		if d.sdel != "" {
			s.delRelation(in.This, d.sdel, pred, labelSelector.String())
		}
		if d.sadd != "" {
			s.addRelation(in.This, d.sadd, pred, labelSelector.String())
		}
	}
	return nil
}

// DeleteNode deletes all references to the node in the graph DB. Removes all edges and vertex.
func (s *cayleyStore) DeleteNode(node string) error {
	if err := s.ctx.Err(); err != nil {
		return err
	}
	defer s.Unlock()
	s.Lock()
	return errors.Wrap(s.writer.RemoveNode(quad.String(node)), fmt.Sprintf("Node:%v", node))
}

// References returns the node with all references populated
func (s *cayleyStore) References(in string) *Node {
	if err := s.ctx.Err(); err != nil {
		return nil
	}
	defer s.Unlock()
	s.Lock()
	return s.getNode(in, RefOut)
}

// Referrers returns the node with all reverese references populated. i.e. all nodes referring to this node.
func (s *cayleyStore) Referrers(in string) *Node {
	if err := s.ctx.Err(); err != nil {
		return nil
	}
	defer s.Unlock()
	s.Lock()
	return s.getNode(in, RefIn)
}

// Tree returns a complete tree of the graph rooted at this node.
// XXX-TBD(sanjayt): handle loops in the graph and raise an error maybe.
func (s *cayleyStore) Tree(in string, dir Direction) *Vertex {
	if err := s.ctx.Err(); err != nil {
		return nil
	}
	defer s.Unlock()
	s.Lock()
	visited := make(map[string]*Vertex)
	// follow in a recursive approach for now
	//  XXX-TBD(sanjayt): Move to a stack based non-recursive approach
	return s.getVertex(in, dir, visited)
}

// IsIsolated returns true if there are no nodes referring to this node.
func (s *cayleyStore) IsIsolated(in string) bool {
	if err := s.ctx.Err(); err != nil {
		return true
	}
	sq := shape.Quads{
		{Dir: quad.Label, Values: shape.Lookup{quad.StringToValue(labelStrict.String())}},
		{Dir: quad.Object, Values: shape.Lookup{quad.StringToValue(in)}},
	}

	defer s.Unlock()
	s.Lock()
	it := shape.BuildIterator(s.store, sq)
	defer it.Close()
	c, _ := it.Size()
	if c == 0 {
		return true
	}
	// Till Shape query can actually work with Labels have to iterate
	for it.Next(context.TODO()) {
		if s.store.Quad(it.Result()).Label.Native().(string) == labelStrict.String() {
			return false
		}
	}
	return true
}

// Close cancels all operations and cleans up the graph store
func (s *cayleyStore) Close() {
	defer s.Unlock()
	s.Lock()
	s.cancel()
	s.store.Close()
}

func (s *cayleyStore) dumpQuads(it graph.Iterator) string {

	ret := fmt.Sprintf("===> dumping Quads\n")
	if it == nil {
		it = s.store.QuadsAllIterator()
	}
	for it.Next(context.TODO()) {
		ret = fmt.Sprintf("%s=> [%+v], label[%v]\n", ret, s.store.Quad(it.Result()), s.store.Quad(it.Result()).Label)
	}
	return ret
}

// Dump dumps all the quads in the DB to output string for debugging
func (s *cayleyStore) Dump(filter string) string {
	return s.dumpQuads(nil)
}
