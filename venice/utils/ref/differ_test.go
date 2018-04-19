package ref

import (
	"testing"

	mapset "github.com/deckarep/golang-set"
)

type nestedStruct struct {
	String string
}

type simplestruct struct {
	Int    int
	String string
	Struct nestedStruct
}

type simpleInterface interface {
	DummyFunc1()
}

type intImpl1 struct{}

func (i *intImpl1) DummyFunc1() {
	return
}

type intImpl2 struct{}

func (i *intImpl2) DummyFunc1() {
	return
}

type strEnum string

const (
	One   strEnum = "one"
	Two   strEnum = "two"
	Three strEnum = "three"
)

type intEnum int

const (
	OneInt   intEnum = iota
	TwoInt   intEnum = iota
	ThreeInt intEnum = iota
)

type strct struct {
	Int       int
	intg      int
	String    string
	IntP      *int
	StringP   *string
	Struct    simplestruct
	StructP   *simplestruct
	Map       map[int]string
	MapS      map[string]simplestruct
	SliceI    []int
	SliceS    []string
	SliceSt   []simplestruct
	Array     [10]simplestruct
	Interface simpleInterface
	StrEnum   strEnum
	IntEnum   intEnum
}

func (s *strct) Init() *strct {
	s.Int = 111
	s.intg = 222
	s.String = "TestVal1"
	t := 333
	s.IntP = &t
	str := "TestString"
	s.StringP = &str
	s.Struct = simplestruct{Int: 321, String: "StructEmbedded"}
	s.StructP = &simplestruct{Int: 322, String: "StructPointer"}
	s.Map = make(map[int]string)
	s.MapS = make(map[string]simplestruct)
	s.Map[1] = "Key1Value"
	s.Map[2] = "Key2Value"
	sstrc1 := simplestruct{Int: 444, String: "Strv1"}
	sstrc2 := simplestruct{Int: 444, String: "Strv1"}
	s.MapS["skey1"] = sstrc1
	s.MapS["skey2"] = sstrc2
	s.SliceI = nil
	s.SliceI = append(s.SliceI, 10, 11, 12)
	s.SliceS = nil
	s.SliceS = append(s.SliceS, "StringExa1", "StringExa3", "StringExa2")
	s.Array = [10]simplestruct{
		{Int: 1, String: "One", Struct: nestedStruct{String: "nested"}},
		{Int: 2, String: "two", Struct: nestedStruct{String: "nested"}},
		{Int: 3, String: "three", Struct: nestedStruct{String: "nested"}},
		{Int: 4, String: "four", Struct: nestedStruct{String: "nested"}},
		{Int: 5, String: "five", Struct: nestedStruct{String: "nested"}},
		{Int: 6, String: "six", Struct: nestedStruct{String: "nested"}},
		{Int: 7, String: "seven", Struct: nestedStruct{String: "nested"}},
		{Int: 8, String: "eight", Struct: nestedStruct{String: "nested"}},
		{Int: 9, String: "nine", Struct: nestedStruct{String: "nested"}},
		{Int: 10, String: "ten", Struct: nestedStruct{String: "nested"}},
	}
	s.Interface = &intImpl1{}
	s.StrEnum = One
	s.IntEnum = OneInt
	return s
}

func TestDiffer2(t *testing.T) {
	var t1, t2 strct
	cases := []struct {
		name    string
		fn      func(s *strct)
		changed bool
		exp     []string
		lookup  []string
		depth   int
	}{
		{
			"No Changes",
			func(s *strct) {},
			false,
			nil,
			nil,
			0,
		},
		{
			"No Changes",
			func(s *strct) {},
			false,
			nil,
			nil,
			4,
		},
		{
			"Integer change",
			func(s *strct) { s.Int = 12222 },
			true,
			[]string{".", "Int"},
			[]string{"Int"},
			0,
		},
		{
			"String change",
			func(s *strct) { s.String = "changed" },
			true,
			[]string{".", "String"},
			[]string{"String"},
			0,
		},
		{
			"String change",
			func(s *strct) { s.String = "changed" },
			true,
			[]string{".", "String"},
			[]string{"String"},
			1,
		},
		{
			"unexported member",
			func(s *strct) { s.intg = 2222222 },
			false,
			nil,
			nil,
			0,
		},
		{
			"Integer Pointer same value",
			func(s *strct) {
				newint := 333
				s.IntP = &newint
			},
			false,
			nil,
			nil,
			0,
		},
		{
			"Integer Pointer changed value",
			func(s *strct) {
				newint := 1313131
				s.IntP = &newint
			},
			true,
			[]string{".", "IntP"},
			[]string{"IntP"},
			0,
		},
		{
			"String Pointer same value",
			func(s *strct) {
				newstr := "TestString"
				s.StringP = &newstr
			},
			false,
			nil,
			nil,
			0,
		},
		{
			"String Pointer changed value",
			func(s *strct) {
				newstr := "Changed String"
				s.StringP = &newstr
			},
			true,
			[]string{".", "StringP"},
			[]string{"StringP"},
			0,
		},
		{
			"Struct changed",
			func(s *strct) {
				s.Struct.Int = 3321
				s.Struct.Struct.String = "nestednow"
			},
			true,
			[]string{".", "Struct", "Struct.Int", "Struct.Struct", "Struct.Struct.String"},
			[]string{"Struct", "Struct.Int", "Struct.Struct", "Struct.Struct.String"},
			0,
		},
		{
			"Struct Pointer changed",
			func(s *strct) { s.StructP.Int = 3321 },
			true,
			[]string{".", "StructP", "StructP.Int"},
			[]string{"StructP", "StructP.Int"},
			0,
		},
		{
			"Map changed value",
			func(s *strct) { s.Map[2] = "ChangedValue" },
			true,
			[]string{".", "Map", "Map[2]"},
			[]string{"Map", "Map[2]"},
			0,
		},
		{
			"Map changed value",
			func(s *strct) { s.Map[2] = "ChangedValue" },
			true,
			[]string{".", "Map"},
			[]string{"Map"},
			1,
		},
		{
			"Map New Key",
			func(s *strct) { s.Map[3] = "NewValue" },
			true,
			[]string{".", "Map"},
			[]string{"Map"},
			0,
		},
		{
			"Map[]Structure Changed Value",
			func(s *strct) { s.MapS["skey1"] = simplestruct{Int: 555, String: "Strv1"} },
			true,
			[]string{".", "MapS[skey1].Int", "MapS", "MapS[skey1]"},
			[]string{"MapS[skey1].Int", "MapS", "MapS[skey1]"},
			0,
		},
		{
			"Map[]Structure Changed Value(depth 3)",
			func(s *strct) { s.MapS["skey1"] = simplestruct{Int: 555, String: "Strv1"} },
			true,
			[]string{".", "MapS[skey1].Int", "MapS", "MapS[skey1]"},
			[]string{"MapS[skey1].Int", "MapS", "MapS[skey1]"},
			3,
		},
		{
			"Map[]Structure Changed Value(depth 2)",
			func(s *strct) { s.MapS["skey1"] = simplestruct{Int: 555, String: "Strv1"} },
			true,
			[]string{".", "MapS", "MapS[skey1]"},
			[]string{"MapS", "MapS[skey1]"},
			2,
		},
		{
			"Map[]Structure new Key",
			func(s *strct) { s.MapS["skey3"] = simplestruct{Int: 555, String: "Strv1"} },
			true,
			[]string{".", "MapS"},
			[]string{"MapS"},
			0,
		},
		{
			"Integer Slice changed value",
			func(s *strct) { s.SliceI[0] = 13 },
			true,
			[]string{".", "SliceI", "SliceI[0]"},
			[]string{"SliceI", "SliceI[0]"},
			0,
		},
		{
			"Integer Slice changed size",
			func(s *strct) { s.SliceI = append(s.SliceI, 13) },
			true,
			[]string{".", "SliceI"},
			[]string{"SliceI"},
			0,
		},
		{
			"Integer Slice nil value",
			func(s *strct) { s.SliceI = nil },
			true,
			[]string{".", "SliceI"},
			[]string{"SliceI"},
			0,
		},
		{
			"String Slice changed value",
			func(s *strct) { s.SliceS[1] = "changed value" },
			true,
			[]string{".", "SliceS", "SliceS[1]"},
			[]string{"SliceS", "SliceS[1]"},
			0,
		},
		{
			"String Slice changed size",
			func(s *strct) { s.SliceS = append(s.SliceS, "new value") },
			true,
			[]string{".", "SliceS"},
			[]string{"SliceS"},
			0,
		},
		{
			"Array changed value",
			func(s *strct) {
				s.Array[8] = simplestruct{Int: 18, String: "eighteen", Struct: nestedStruct{String: "nested"}}
			},
			true,
			[]string{".", "Array[8].Int", "Array[8].String", "Array[8]", "Array"},
			[]string{"Array[8].Int", "Array[8].String", "Array[8]", "Array"},
			0,
		},
		{
			"Array changed value(depth 4)",
			func(s *strct) {
				s.Array[8] = simplestruct{Int: 18, String: "eighteen", Struct: nestedStruct{String: "nested"}}
			},
			true,
			[]string{".", "Array[8].Int", "Array[8].String", "Array[8]", "Array"},
			[]string{"Array[8].Int", "Array[8].String", "Array[8]", "Array"},
			4,
		},
		{
			"Array changed value(depth 2)",
			func(s *strct) {
				s.Array[8] = simplestruct{Int: 18, String: "eighteen", Struct: nestedStruct{String: "nested"}}
			},
			true,
			[]string{".", "Array[8]", "Array"},
			[]string{"Array[8]", "Array"},
			2,
		},
		{
			"Changed interface",
			func(s *strct) { s.Interface = &intImpl2{} },
			true,
			[]string{".", "Interface"},
			[]string{"Interface"},
			0,
		},
		{
			"Nil interface",
			func(s *strct) { s.Interface = nil },
			true,
			[]string{".", "Interface"},
			[]string{"Interface"},
			0,
		},
		{
			"String Enum",
			func(s *strct) { s.StrEnum = Two },
			true,
			[]string{".", "StrEnum"},
			[]string{"StrEnum"},
			0,
		},
		{
			"Int Enum",
			func(s *strct) { s.IntEnum = TwoInt },
			true,
			[]string{".", "IntEnum"},
			[]string{"IntEnum"},
			0,
		},
		{
			"Multiple changes",
			func(s *strct) {
				s.Array[8] = simplestruct{18, "eighteen", nestedStruct{"nested"}}
				s.SliceS[1] = "changed value"
				s.SliceS = append(s.SliceS, "new value")
				s.Map[2] = "ChangedValue"
			},
			true,
			[]string{".", "Array[8].Int", "Array[8].String", "Array[8]", "Array", "SliceS", "Map", "Map[2]"},
			[]string{"Array[8].Int", "Array[8].String", "Array[8]", "Array", "SliceS", "Map", "Map[2]"},
			0,
		},
		{
			"Multiple changes(depth 2)",
			func(s *strct) {
				s.Array[8] = simplestruct{18, "eighteen", nestedStruct{"nested"}}
				s.SliceS[1] = "changed value"
				s.SliceS = append(s.SliceS, "new value")
				s.Map[2] = "ChangedValue"
			},
			true,
			[]string{".", "Array", "Array[8]", "SliceS", "Map", "Map[2]"},
			[]string{"Array", "SliceS", "Map"},
			2,
		},
	}
	for _, c := range cases {
		t1.Init()
		t2.Init()
		c.fn(&t1)
		var opts []Option
		if c.depth != 0 {
			opts = append(opts, WithMaxDepth(c.depth))
		}
		diff, ok := ObjDiff(t1, t2, opts...)
		list := diff.List()
		if c.exp != nil && !ok {
			t.Fatalf("--%s  : expecting no diff buf found [%v]", c.name, list)
		}
		if ok != c.changed {
			t.Fatalf("--%s  : changed - expecting [%v] got [%v]", c.name, c.changed, ok)
		}
		if ok && len(list) != len(c.exp) {
			t.Fatalf("-%s : expecting [%v] diff got [%v]", c.name, c.exp, list)
		}
		if ok && len(list) > 0 {
			var e, a []interface{}
			for i := range c.exp {
				e = append(e, c.exp[i])
			}
			for i := range list {
				a = append(a, list[i])
			}
			if !mapset.NewSetFromSlice(e).Equal(mapset.NewSetFromSlice(a)) {
				t.Fatalf("-%s : expecting [%v] diff got [%v]", c.name, c.exp, list)
			}
			for i := range c.lookup {
				if ok := diff.Lookup(c.lookup[i]); !ok {
					t.Errorf("-%s : Expecting lookup to pass for [%s]", c.name, c.lookup[i])
				}
			}
		}
	}
}

func TestDifferentTypes(t *testing.T) {
	var t1 strct
	t1.Init()
	t2 := struct {
		dummy string
	}{}
	diff, ok := ObjDiff(t1, t2)
	if !ok {
		t.Fatalf("did not find diff for different types")
	}
	got := diff.List()
	if len(got) != 1 {
		t.Fatalf("expecting {.}, got %v", got)
	}
	if got[0] != "." {
		t.Fatalf("expecting {.}, got %v", got)
	}
	// Lookup random path
	if !diff.Lookup("Any.Path") {
		t.Fatalf("expecing true got false")
	}
}

func TestUninitialized(t *testing.T) {
	var t1, t2 strct
	t1.Init()
	_, ok := ObjDiff(t1, t2)
	if !ok {
		t.Fatalf("did not find diff")
	}
	var t3 strct
	_, ok = ObjDiff(t2, t3)
	if ok {
		t.Fatalf("found change in uninitialized")
	}
}

func BenchmarkDiffer(b *testing.B) {
	var t1, t2 strct
	t1.Init()
	t2.Init()
	for i := 0; i < b.N; i++ {
		ObjDiff(t1, t2)
	}
}

func BenchmarkDifferWithChange(b *testing.B) {
	var t1, t2 strct
	t1.Init()
	t2.Init()
	t1.Struct.Int = 3321
	t1.Struct.Struct.String = "nestednow"
	for i := 0; i < b.N; i++ {
		ObjDiff(t1, t2)
	}
}

func BenchmarkDifferWithMaxDepth(b *testing.B) {
	var t1, t2 strct
	t1.Init()
	t2.Init()
	t1.Struct.Int = 3321
	t1.Struct.Struct.String = "nestednow"
	for i := 0; i < b.N; i++ {
		ObjDiff(t1, t2, WithMaxDepth(1))
	}
}
