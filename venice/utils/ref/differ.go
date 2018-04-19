// Deep differ using reflection
// Inspired by DeepEqual from reflect package

package ref

import (
	"fmt"
	"reflect"
	"regexp"
	"strings"
	"unsafe"
)

const (
	// subscriptsRegex is used for regular expression for finding subsscripts
	subscriptsRegex = "([a-zA-Z0-9_\\-]*)*\\[([a-zA-Z0-9_\\-]*)*\\]"
	// defaultMaxDepth is used as max depth when no max depth is specified by user.
	defaultMaxDepth = 32
)

var (
	subscriptRe = regexp.MustCompile(subscriptsRegex)
)

// visit keeps track of checks that are
// in progress. The comparison algorithm assumes that all
// checks in progress are true when it reencounters them.
// Visited comparisons are stored in a map indexed by visit.
type visit struct {
	a1  unsafe.Pointer
	a2  unsafe.Pointer
	typ reflect.Type
}

// options for the ObjDiffer
type options struct {
	// maxDepth specifies how deep to check. default Depth is 32. If depth is specified
	//  the ObjDiff only finds diffs to the depth specified.
	maxDepth int
}

// Elem is each node of the diff.
type Elem struct {
	Field  string
	Parent string
	Elems  map[string]Elem
}

// makeFieldName generates the field name given the parent and the field
func makeFieldName(parent, delimiter, field string) string {
	str := ""
	if parent == "<root>" || parent == "." {
		str = fmt.Sprintf("%s", field)
	} else if field == "<root>" {
		str = fmt.Sprintf(".")
	} else {
		str = fmt.Sprintf("%s%s%s", parent, delimiter, field)
	}
	return str
}

func (e *Elem) listDiff(l []string) []string {
	str := makeFieldName(e.Parent, ".", e.Field)
	if len(e.Field) != 0 {
		// When using a subscripted field like a slice or a map, do not delimit with a "."
		//  Map[Key] rather than Map.[Key]
		if string(e.Field[0]) == "[" {
			str = makeFieldName(e.Parent, "", e.Field)
		}
	}
	l = append(l, str)

	for _, v := range e.Elems {
		l = v.listDiff(l)
	}
	return l
}

// Option is used to pass options to ObjDiff
type Option func(*options)

// WithMaxDepth option restricts the differ to a max depth. The differ bails
//  after the first change is found. Results only have diffs upto that depth.
func WithMaxDepth(depth int) Option {
	return func(i *options) {
		i.maxDepth = depth
	}
}

// Diffs is the result of the Differ
type Diffs struct {
	Root Elem
	re   *regexp.Regexp
}

// List lists all paths where a diff was found as a slice of strings
func (d *Diffs) List() []string {
	var diffs []string
	return d.Root.listDiff(diffs)
}

// Lookup examines if there is a diff at the path given or below. Returns true if there is a diff
func (d *Diffs) Lookup(path string) bool {
	c := strings.Split(path, ".")
	e := d.Root
	if len(e.Elems) == 0 {
		return true
	}
	ok := true
	for i := range c {
		if params := d.re.FindStringSubmatch(c[i]); params != nil {
			if len(params) != 3 {
				return false
			}
			params[2] = "[" + params[2] + "]"
			for _, v := range params[1:] {
				if e, ok = e.Elems[v]; !ok {
					return false
				}
			}
		} else {
			if e, ok = e.Elems[c[i]]; !ok {
				return false
			}
		}
	}
	return true
}

// deepDiff recursively walks the objects v1 and v2 to find diffs in the objects. The diffs are accumulated in cur
//  return true if the objects are the same.
func deepDiff(v1, v2 reflect.Value, visited map[visit]bool, opts *options, depth int, cur Elem, name, parent string) bool {
	equal := true
	ok := true
	if !v1.IsValid() || !v2.IsValid() {
		// If one of them is valid and not the other, no need for further checking.
		if v1.IsValid() != v2.IsValid() {
			cur.Elems[name] = Elem{Elems: make(map[string]Elem), Field: name, Parent: parent}
			return false
		}
		return true
	}
	if v1.Type() != v2.Type() {
		cur.Elems[name] = Elem{Elems: make(map[string]Elem), Field: name, Parent: parent}
		return false
	}

	// We want to avoid putting more in the visited map than we need to.
	// For any possible reference cycle that might be encountered,
	// hard(t) needs to return true for at least one of the types in the cycle.
	hard := func(k reflect.Kind) bool {
		switch k {
		case reflect.Map, reflect.Slice, reflect.Ptr, reflect.Interface:
			return true
		}
		return false
	}

	if v1.CanAddr() && v2.CanAddr() && hard(v1.Kind()) {
		addr1 := unsafe.Pointer(v1.UnsafeAddr())
		addr2 := unsafe.Pointer(v2.UnsafeAddr())
		if uintptr(addr1) > uintptr(addr2) {
			// Canonicalize order to reduce number of entries in visited.
			// Assumes non-moving garbage collector.
			addr1, addr2 = addr2, addr1
		}

		// Short circuit if references are already seen.
		typ := v1.Type()
		v := visit{addr1, addr2, typ}
		if visited[v] {
			return true
		}

		// Remember for later.
		visited[v] = true
	}
	delimiter := "."
	if strings.HasPrefix(name, "[") {
		delimiter = ""
	}
	elem := Elem{Elems: make(map[string]Elem), Field: name, Parent: parent}
	switch v1.Kind() {
	case reflect.Array:
		same := true
		p := makeFieldName(parent, ".", name)
		for i := 0; i < v1.Len(); i++ {
			ok = deepDiff(v1.Index(i), v2.Index(i), visited, opts, depth+1, elem, fmt.Sprintf("[%d]", i), p)
			if !ok {
				if depth > opts.maxDepth {
					return false
				}
				same = false
			}
		}
		equal = same
		if !equal {
			cur.Elems[name] = elem
			return false
		}
		return true
	case reflect.Slice:
		if v1.IsNil() != v2.IsNil() {
			if depth <= opts.maxDepth {
				cur.Elems[name] = elem
			}
			return false
		}
		if v1.Len() != v2.Len() {
			if depth <= opts.maxDepth {
				cur.Elems[name] = elem
			}
			return false
		}
		if v1.Pointer() == v2.Pointer() {
			return true
		}
		same := true
		p := makeFieldName(parent, delimiter, name)
		for i := 0; i < v1.Len(); i++ {
			ok = deepDiff(v1.Index(i), v2.Index(i), visited, opts, depth+1, elem, fmt.Sprintf("[%d]", i), p)
			if !ok {
				if depth > opts.maxDepth {
					return false
				}
				same = false
			}
		}
		equal = same
		if !equal {
			cur.Elems[name] = elem
			return false
		}
		return true
	case reflect.Interface:
		if v1.IsNil() || v2.IsNil() {
			if v1.IsNil() == v2.IsNil() {
				return true
			}
			if depth <= opts.maxDepth {
				cur.Elems[name] = elem
			}
			return false
		}
		if ok := deepDiff(v1.Elem(), v2.Elem(), visited, opts, depth+1, cur, name, parent); !ok {
			return false
		}
		return true
	case reflect.Ptr:
		if v1.Pointer() == v2.Pointer() {
			return true
		}
		if ok = deepDiff(v1.Elem(), v2.Elem(), visited, opts, depth, cur, name, parent); !ok {
			return false
		}
		return true
	case reflect.Struct:
		same := true
		p := makeFieldName(parent, delimiter, name)
		for i, n := 0, v1.NumField(); i < n; i++ {
			if ok = deepDiff(v1.Field(i), v2.Field(i), visited, opts, depth+1, elem, fmt.Sprintf("%s", v1.Type().Field(i).Name), p); !ok {
				if depth > opts.maxDepth {
					return false
				}
				same = false
			}
		}
		if !same {
			cur.Elems[name] = elem
			return false
		}
		return true
	case reflect.Map:
		if v1.IsNil() != v2.IsNil() {
			if depth <= opts.maxDepth {
				cur.Elems[name] = elem
			}
			return false
		}
		if v1.Len() != v2.Len() {
			if depth <= opts.maxDepth {
				cur.Elems[name] = elem
			}
			return false
		}
		if v1.Pointer() == v2.Pointer() {
			return true
		}
		same := true
		p := makeFieldName(parent, delimiter, name)
		for _, k := range v1.MapKeys() {
			val1 := v1.MapIndex(k)
			val2 := v2.MapIndex(k)
			if !val1.IsValid() || !val2.IsValid() {
				if depth <= opts.maxDepth {
					elem.Elems[fmt.Sprintf("%v", k)] = Elem{Elems: make(map[string]Elem), Field: fmt.Sprintf("[%v]", k), Parent: p}
					same = false
				} else {
					return false
				}
			}
			if ok = deepDiff(v1.MapIndex(k), v2.MapIndex(k), visited, opts, depth+1, elem, fmt.Sprintf("[%v]", k), p); !ok {
				if depth > opts.maxDepth {
					return false
				}
				same = false
			}
		}
		if !same {
			cur.Elems[name] = elem
			return false
		}
		return true
	case reflect.Func:
		if v1.IsNil() && v2.IsNil() {
			return true
		}
		if depth <= opts.maxDepth {
			cur.Elems[name] = elem
		}
		// Can't do better than this:
		return false
	default:
		// Normal equality suffices
		if v1.CanInterface() && v2.CanInterface() {
			if v1.Interface() == v2.Interface() {
				return true
			}
			if depth <= opts.maxDepth {
				cur.Elems[name] = elem
			}
			return false
		}
		return true
	}
}

// ObjDiff returns a Diffs object with diffs and a bool that is set to true if there is a diff.
//  The returned Diff is typically used through accessor functions Lookup() and List()
func ObjDiff(x, y interface{}, opts ...Option) (Diffs, bool) {
	ret := Diffs{re: subscriptRe}
	if x == nil || y == nil {
		return ret, !(x == nil && y == nil)
	}
	o := options{
		maxDepth: defaultMaxDepth,
	}
	for _, fn := range opts {
		fn(&o)
	}
	v1 := reflect.ValueOf(x)
	v2 := reflect.ValueOf(y)
	if v1.Type() != v2.Type() {
		return ret, true
	}
	root := Elem{Elems: make(map[string]Elem), Field: "<root>", Parent: ""}
	if deepDiff(v1, v2, make(map[visit]bool), &o, 0, root, "<root>", "") {
		return ret, false
	}
	ret.Root = root.Elems["<root>"]
	return ret, true
}
