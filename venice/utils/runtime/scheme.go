package runtime

import (
	"errors"
	"fmt"
	"reflect"
	"strings"
	"sync"

	"github.com/gogo/protobuf/protoc-gen-gogo/descriptor"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	defaultScheme *Scheme
	once          sync.Once
)

type scopes struct {
	tenant, cluster bool
}

// Scheme contains methods to help with serialization/deserialization of API
// objects.
type Scheme struct {
	kindToTypes map[string]reflect.Type
	Types       map[string]*api.Struct
	group2Kinds map[string][]string
	kind2Type   map[string]string
	kind2Group  map[string]string
	kind2Scopes map[string]scopes
	procOnce    sync.Once
	paths       map[string][]api.PathsMap
	keyTouri    *pathNode
	uriToKey    *pathNode
}

// NewScheme returns a new Scheme.
func NewScheme() *Scheme {
	return &Scheme{
		kindToTypes: make(map[string]reflect.Type),
		Types:       make(map[string]*api.Struct),
		group2Kinds: make(map[string][]string),
		kind2Type:   make(map[string]string),
		kind2Group:  make(map[string]string),
		kind2Scopes: make(map[string]scopes),
		paths:       make(map[string][]api.PathsMap),
	}
}

// AddKnownTypes is used to register known API objects. All API objects need to
// register here.
// TODO: Version based differentiation.
func (s *Scheme) AddKnownTypes(types ...Object) {
	for _, obj := range types {
		t := reflect.TypeOf(obj)
		if t.Kind() != reflect.Ptr {
			panic("Must be a ptr")
		}
		t = t.Elem()
		if t.Kind() != reflect.Struct {
			panic("Must be a ptr to struct")
		}
		s.kindToTypes[strings.ToLower(t.Name())] = t
	}
}

// New creates a new object given a kind, if the kind was registered using
// AddKnownTypes above.
func (s *Scheme) New(kind string) (Object, error) {
	kind = strings.ToLower(kind)
	t, exists := s.kindToTypes[kind]
	if !exists {
		return nil, fmt.Errorf("Kind %s not registered with this scheme", kind)
	}
	return reflect.New(t).Interface().(Object), nil
}

// AddSchema adds a set of schema nodes. This is typically added by
//  the generated code during init
func (s *Scheme) AddSchema(in map[string]*api.Struct) {
	for k, v := range in {
		if _, ok := s.Types[k]; ok {
			panic(fmt.Sprintf("type %v already registered", k))
		}
		if v.Kind != "" && v.APIGroup != "" {
			// We cannot have duplicate kinds being registered, so insert blindly.
			s.group2Kinds[v.APIGroup] = append(s.group2Kinds[v.APIGroup], v.Kind)
			s.kind2Type[v.Kind] = k
			s.kind2Group[v.Kind] = v.APIGroup
			scps := scopes{}
			for i := range v.Scopes {
				switch v.Scopes[i] {
				case "Tenant":
					scps.tenant = true
				case "Cluster":
					scps.cluster = true
				}
			}
			s.kind2Scopes[v.Kind] = scps
		}
		s.Types[k] = v
		if v.Tags == nil {
			v.Tags = make(map[string]string)
		}
		for _, f := range v.Fields {
			if f.JSONTag != "" {
				v.Tags[f.JSONTag] = f.Name
			} else {
				v.Tags[f.Name] = f.Name
			}
		}
	}
}

type pathActions string

const (
	// FieldConsume is action to consume the next token
	FieldConsume pathActions = "FieldConsume"
	// NOPAdvance is action to parse and advance to nexxt token
	NOPAdvance pathActions = "NOPAdvance"
)

type pathNode struct {
	actions  []pathActions
	variable []string
	isLeaf   bool
	next     map[string]*pathNode
	output   map[int]string
}

var debugPath bool

func (s *Scheme) mapPath(start *pathNode, in string, prime map[string]string) (string, error) {
	passNo := 0
	curNode := start
	if curNode == nil {
		return "", fmt.Errorf("node is nil")
	}
	vars := make(map[string]string)
	for k, v := range prime {
		vars[k] = v
	}
	tokens := strings.Split(strings.TrimPrefix(in, "/"), "/")
	for _, t := range tokens {
		if debugPath {
			fmt.Printf("path [%v] parsing token %v\n", in, t)
		}
		if len(curNode.actions) > 0 && passNo < len(curNode.actions) && curNode.actions[passNo] == FieldConsume {
			vars[curNode.variable[passNo]] = t
			passNo++
		} else {
			n, ok := curNode.next[t]
			if !ok {
				return "", fmt.Errorf("unknown path entry %v [%+v]", t, curNode)
			}
			if debugPath {
				fmt.Printf("path [%v] jumping to new node %v\n", in, t)
			}
			curNode = n
			passNo = 0
		}
	}
	if debugPath {
		fmt.Printf("Final node is [%v]\n", dumpPaths(curNode, "  "))
	}
	if curNode.isLeaf {
		if debugPath {
			fmt.Printf("path [%v] curNode is leaf pass[%d]  with output [%v]\n", in, passNo, curNode.output)
		}
		ret := curNode.output[passNo]
		for k, v := range vars {
			p := "{" + k + "}"
			if strings.Contains(ret, p) {
				ret = strings.Replace(ret, p, v, -1)
			}
		}
		return ret, nil
	}
	return "", fmt.Errorf("end node is not a leaf [%v/%v]", passNo, curNode.isLeaf)
}

func dumpPaths(p *pathNode, prefix string) string {
	ret := fmt.Sprintf("\n%s{\n", prefix)
	ret = fmt.Sprintf("%s%s Actions: %v]\n", ret, prefix, p.actions)
	ret = fmt.Sprintf("%s%s Variables: %v\n", ret, prefix, p.variable)
	ret = fmt.Sprintf("%s%s IsLeaf [%v] Output[%v]\n", ret, prefix, p.isLeaf, p.output)
	ret = fmt.Sprintf("%s%s Next {\n", ret, prefix)
	for k, v := range p.next {
		ret = fmt.Sprintf("%s%s [\"%v\"] : { %s\n%s  }", ret, prefix, k, dumpPaths(v, prefix+"  "), prefix)
	}
	ret = fmt.Sprintf("%s%s  }}", ret, prefix)
	return ret
}

func (s *Scheme) parsePathTokens(start *pathNode, in, output string) {
	passNo := 0
	curNode := start
	tokens := strings.Split(strings.TrimPrefix(in, "/"), "/")
	for _, t := range tokens {
		if strings.Contains(t, "{") || strings.Contains(t, "}") {
			// This is a variable.
			if strings.HasPrefix(t, "{") && strings.HasSuffix(t, "}") {
				if len(curNode.variable) > passNo {
					curNode.variable[passNo] = strings.TrimPrefix(strings.TrimSuffix(t, "}"), "{")
					curNode.actions[passNo] = FieldConsume
					passNo++
				} else {
					if len(curNode.variable) > passNo {
						curNode.variable[passNo] = strings.TrimPrefix(strings.TrimSuffix(t, "}"), "{")
						curNode.actions[passNo] = FieldConsume
					} else {
						curNode.variable = append(curNode.variable, strings.TrimPrefix(strings.TrimSuffix(t, "}"), "{"))
						curNode.actions = append(curNode.actions, FieldConsume)
					}
					passNo++
				}
			} else {
				panic(fmt.Sprintf("Unexpected path with {} not entirely contained in token[%s]", in))
			}
		} else {
			if n, ok := curNode.next[t]; ok {
				curNode = n
			} else {
				n = &pathNode{
					next:   make(map[string]*pathNode),
					output: make(map[int]string),
				}
				curNode.next[t] = n
				curNode = n
				passNo = 0
			}
		}
	}
	curNode.isLeaf = true
	curNode.output[passNo] = output
}

func (s *Scheme) preProcessPaths() {
	log.Infof("Processing paths [%v]", len(s.paths))
	s.keyTouri = &pathNode{next: make(map[string]*pathNode), output: make(map[int]string)}
	s.uriToKey = &pathNode{next: make(map[string]*pathNode), output: make(map[int]string)}

	for _, v := range s.paths {
		for _, v1 := range v {
			s.parsePathTokens(s.keyTouri, v1.Key, v1.URI)
			s.parsePathTokens(s.uriToKey, v1.URI, v1.Key)
		}
	}
}

// GetKey maps a URI to a kvstore key
func (s *Scheme) GetKey(in string) string {
	s.procOnce.Do(s.preProcessPaths)
	r, err := s.mapPath(s.uriToKey, in, nil)
	if err == nil {
		return r
	}
	log.Errorf("could not map Key for [%v](%s)", in, err)
	return ""
}

// GetURI maps a kvstore key to an URI
func (s *Scheme) GetURI(in, version string) string {
	s.procOnce.Do(s.preProcessPaths)
	verMap := map[string]string{"version": version}
	r, err := s.mapPath(s.keyTouri, in, verMap)
	if err == nil {
		return r
	}
	log.Errorf("could not map URI for [%v](%s)", in, err)
	return ""
}

// AddPaths adds to the paths map
func (s *Scheme) AddPaths(in map[string][]api.PathsMap) {
	for k, v := range in {
		s.paths[k] = append(s.paths[k], v...)
	}
}

// GetSchema returns the type schema for kind specified in in
func (s *Scheme) GetSchema(in string) *api.Struct {
	if val, ok := s.Types[in]; ok {
		return val
	}
	return nil
}

// Kinds returns all the known Kinds grouped by API Group.
func (s *Scheme) Kinds() map[string][]string {
	return s.group2Kinds
}

// Kind2SchemaType returns the schema kind that can be use in GetSchema() given the Kind
func (s *Scheme) Kind2SchemaType(in string) string {
	return s.kind2Type[in]
}

// Kind2APIGroup returns the API group given the Kind
func (s *Scheme) Kind2APIGroup(in string) string {
	return s.kind2Group[in]
}

// IsTenantScoped returns true if the Kind is tenant scoped.
func (s *Scheme) IsTenantScoped(kind string) (bool, error) {
	if v, ok := s.kind2Scopes[kind]; ok {
		return v.tenant, nil
	}
	return false, errors.New("not found")
}

// IsClusterScoped returns true if the kind has cluster wide scope.
func (s *Scheme) IsClusterScoped(kind string) (bool, error) {
	if v, ok := s.kind2Scopes[kind]; ok {
		return v.cluster, nil
	}
	return false, errors.New("not found")
}

// NewEmpty returns a new object of the same kind as the input
func NewEmpty(in Object) (Object, error) {
	val := reflect.ValueOf(in)
	if !val.IsValid() {
		return nil, fmt.Errorf("invalid input")
	}
	if val.Kind() == reflect.Ptr {
		if val.IsNil() {
			return nil, fmt.Errorf("invalid input")
		}
		tpe := reflect.Indirect(val).Type()
		return reflect.New(tpe).Interface().(Object), nil
	}
	tpe := val.Type()
	return reflect.Indirect(reflect.New(tpe)).Interface().(Object), nil
}

// GetDefaultScheme retrieves the default scheme if there is already one or creates one.
func GetDefaultScheme() *Scheme {
	once.Do(func() {
		defaultScheme = NewScheme()
		// Add the default Schema from api
		defaultScheme.AddSchema(api.GetLocalSchema())

	})
	return defaultScheme
}

// IsScalar returns if the provided type is a scalar.
func IsScalar(in string) bool {
	if in == "TYPE_GROUP" || in == "TYPE_MESSAGE" {
		return false
	}
	if in == "api.Timestamp" {
		return true
	}
	_, ok := descriptor.FieldDescriptorProto_Type_value[in]
	return ok
}

// ResetDefaultSchema clears the default schema. Used for tests only.
func ResetDefaultSchema() {
	defaultScheme = NewScheme()
	// Add the default Schema from api
	defaultScheme.AddSchema(api.GetLocalSchema())
}
