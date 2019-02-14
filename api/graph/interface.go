package graph

// Node is a node in the graph minus the edges
type Node struct {
	// This is the id for the node
	This string
	// Dir is the direction of the perspective for the node. It is either
	//  looking at references to this Object (RefIn) or objects this object
	//  refers to (RefOut)
	Dir Direction
	// List of strict references. The map is keyed by the field identifier that
	//  is the cause for the reference (eg. Spec.Workload)
	Refs map[string][]string
	// List of weak references which are not used for enforcing references
	//  checks
	WeakRefs     map[string][]string
	SelectorRefs map[string][]string
}

// Vertex is a node in the graph along with edges
type Vertex struct {
	// This is the id for the node
	This string
	// Dir is the direction of the perspective for the node. It is either
	//  looking at references to this Object (RefIn) or objects this object
	//  refers to (RefOut)
	Dir Direction
	// List of strict references. The map is keyed by the field identifier that
	//  is the cause for the reference (eg. Spec.Workload)
	Refs map[string][]*Vertex
	// List of weak references which are not used for enforcing references
	//  checks
	WeakRefs map[string][]*Vertex
	// SelectorRefs points to selector. Objects are resolved just in time.
	SelectorRefs map[string][]string
}

// Direction specifies the direction of the relation
type Direction string

const (
	// RefIn is reference from Objects perspective
	RefIn = Direction("ReferenceIn")
	// RefOut is reference from Subjects perspective
	RefOut = Direction("ReferenceOut")
)

// Interface defines interface for the graph object
type Interface interface {
	// Graph update operations
	UpdateNode(in *Node) error
	DeleteNode(node string) error

	// Reference query operations
	References(in string) *Node
	Referrers(in string) *Node

	// Tree returns a resolved directed graph with this node as the root. The tree can be walked
	//  without further calls to the graph.
	Tree(in string, dir Direction) *Vertex

	// IsIsolated returns true if there are no objects referring to this object
	IsIsolated(in string) bool

	// Dump is used for debugging and dumps the graph DB. Filter string filters the dump.
	Dump(filter string) string

	Close()
}
