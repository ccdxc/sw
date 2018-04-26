package fields

import (
	"testing"
)

func TestFieldKeyValidation(t *testing.T) {
	goodKeys := []string{
		"ab.cd",        // two level
		"a.b.c.d",      // multi level
		"a.b[*].cd",    // Slice field (any index)
		"a.b[x].cd",    // Map field (with key x)
		"ab.cd[*]",     // Slice leaf
		"ab.cd[x]",     // Map leaf
		"a.b[x].cd[*]", // Map with Slice leaf
		"a.b[x].cd[y]", // Map with Map leaf
		"a.b[*].cd[y]", // Slice with Map leaf
		"a.b[*].cd[*]", // Slice with Slice leaf
	}
	badKeys := []string{
		"abcd",        // Has to be a atleast two level
		"abcd.",       // Invalid
		"ab[cd",       // Invalid
		"ab[]cd",      // Invalid
		"ab*cd",       // Invalid
		"ab[*]cd",     // Invalid
		"ab0.cd",      // Invalid, no numerics in name
		"ab*.cd",      // Invalid
		"a.b[x].cd[]", // Invalid
		"a.b[x].cd]",  // Invalid
	}
	for ii := range goodKeys {
		if err := validateFieldKey(goodKeys[ii]); err != nil {
			t.Fatalf("%v", err)
		}
	}
	for ii := range badKeys {
		if err := validateFieldKey(badKeys[ii]); err == nil {
			t.Fatalf("Validation success, should have failed: %v", badKeys[ii])
		}
	}
}

func TestFieldOpValidation(t *testing.T) {
	goodOps := []string{
		" in ",
		" notin ",
		"=",
		"!=",
		" =",
		"  =",
		"= ",
		"=  ",
		" = ",
		"  !=",
		"   in   ",
		" notin   ",
		"\tnotin\t",
	}
	badOps := []string{
		"in",    // no spaces
		"notin", // no spaces
		" in",
		"in ",
		"in\t",
		"inn",
		"innotin",
		"==",
		"=!",
		"!==",
		"!in",
	}
	for ii := range goodOps {
		if err := validateFieldOp(goodOps[ii]); err != nil {
			t.Fatalf("%v", err)
		}
	}
	for ii := range badOps {
		if err := validateFieldOp(badOps[ii]); err == nil {
			t.Fatalf("Validation success, should have failed: %v", badOps[ii])
		}
	}
}

func TestFieldValsValidation(t *testing.T) {
	goodVals := []string{
		"",
		"in",
		"notin",
		"abc",
		"abc\\,def",
		"abc def",
		" abc   def",
		"1000",
		"a/b",
		"a-b",
		"a_b",
		"a.b",
		"(abc)",
		"(abc,def)",
		"(abc,100)",
		"(100,200,300)",
		"(abc\\,def,def)",
	}
	badVals := []string{
		"()",
		"a,b,",
		",a",
		")a",
		"(a",
		"(a,)",
		"(,a)",
		"(a,,b)",
	}
	for ii := range goodVals {
		if err := validateFieldVals(goodVals[ii]); err != nil {
			t.Fatalf("%v", err)
		}
	}
	for ii := range badVals {
		if err := validateFieldVals(badVals[ii]); err == nil {
			t.Fatalf("Validation success, should have failed: %v", badVals[ii])
		}
	}
}

func TestSelectorValidation(t *testing.T) {
	goodSels := []string{
		"x.a=a,y.b=b,z.c=  c",
		"x.a=a b,y.b=b,z.c=  c",
		"x.a=a b c,y.b=b,z.c=  c",
		"x.c!=a,y.c=b",
		"x.y in (a,b,c)",
		"x.x in   (a,b),y.y in (c,d)",
		"x.x notin (a,b,c)",
		"x.x in (a,b),y.y   notin (a,b)",
		"x.x=",
		"x.x= ",
		"x.x=,z.z= ",
		"x.x= ,z.z= ",
		"x.x= something good,z.z= ",
		"x.x= something very good,z.z= ",
	}
	badSels := []string{
		"",
		"x.x=a||y.y=b",
		"x.x==a==b",
		"!x.x=a",
		"x.x<a",
		"!x.x",
		"x.x>1",
		"x.x>1,z<5",
		"x.x in a",
		"x.x in (a",
		"x.x in (a,b",
		"x.x=(a,b)",
	}
	for ii := range goodSels {
		if err := validateSelector(goodSels[ii]); err != nil {
			t.Fatalf("%v", err)
		}
	}
	for ii := range badSels {
		if err := validateSelector(badSels[ii]); err == nil {
			t.Fatalf("Validation success, should have failed: %v", badSels[ii])
		}
	}
}
