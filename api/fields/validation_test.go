package fields

import (
	"testing"
)

func TestFieldKeyValidation(t *testing.T) {
	goodKeys := []string{
		"ab.cd",               // two level
		"a.b.c.d",             // multi level
		"a.b[*].cd",           // Map field - key (any)
		"a.b[x].cd",           // Map field - key (x)
		"ab.cd[*]",            // Map leaf - key (any)
		"ab.cd[x]",            // Map leaf - key (x)
		"a.b[x].cd[*]",        // Map with Map leaf - keys (x, any)
		"a.b[x].cd[y]",        // Map with Map leaf - keys (x, y)
		"a.b[*].cd[y]",        // Map with Map leaf - keys (any, y)
		"a.b[*].cd[*]",        // Map with Map leaf - keys (any, any)
		"a.b[0].cd[*]",        // Map with Map leaf - keys (0, any)
		"a.b[x].cd[9999]",     // Map with Map leaf - keys (x, 9999)
		"x.a-b",               // - is valid
		"x.a-b-c.d-e-f",       // - is valid
		"x.a-b-c[*].d-e-f",    // - is valid
		"x.a-b-c[*].d-e-f[x]", // - is valid
		"x.ABC[x].d-e-f[*]",   // upper case is valid
	}
	badKeys := []string{
		"abcd",         // Has to be a atleast two level
		"abcd.",        // Invalid
		"ab[cd",        // Invalid
		"ab[]cd",       // Invalid
		"ab.cd[",       // Invalid
		"ab.cd[].",     // Invalid
		"ab.cd[*].",    // Invalid
		"ab.cd[*]*",    // Invalid
		"ab.cd[[]",     // Invalid
		"ab.cd[[*]]",   // Invalid
		"ab*cd",        // Invalid
		"ab[*]cd",      // Invalid
		"ab0.cd",       // Invalid, no numerics in name
		"ab*.cd",       // Invalid
		"a.b[x].cd[]",  // Invalid
		"a.b[x].cd]",   // Invalid
		"a.b[11.1].cd", // Invalid
		"x.a-b-",       // cant end in -
		"x-.ab",        // cant end in -
		"x.-a-b",       // cant begin in -
		"-x.ab",        // cant being in -
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
		"abc   def",
		"1000",
		"a/b",
		"a-b",
		"a_b",
		"a.b",
		"a.b.c",
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
		"a.b[*].cd=x",
		"a.b[x].cd!=y",
		"ab.cd[*] in (x,y)",
		"ab.cd[x] notin (x)",
		"a.b[x].cd[*]=x",
		"a.b[x].cd[y]= y",
		"a.b[*].cd[y] in   (x,z)",
		"a.b[*].cd[*] notin\t (x,z)",
		"a.b[10].cd[y] in   (x,z)",
		"a.b[*].cd[8787] notin\t (x,z)",
	}
	badSels := []string{
		"",
		"x.x==",
		"x.x[*]==",
		"x.x[*]*=",
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
		"a.b[999.99].cd[x] in (x,z)",
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

func TestCheckVal(t *testing.T) {
	tests := []struct {
		kind       string
		value      string
		expSuccess bool
	}{
		{"TYPE_STRING", "abc", true},
		{"TYPE_BOOL", "true", true},
		{"TYPE_BOOL", "abc", false},
		{"TYPE_FLOAT", "100.10", true},
		{"TYPE_FLOAT", "abc", false},
		{"TYPE_INT64", "123123213123", true},
		{"TYPE_INT64", "-123123213123", true},
		{"TYPE_UINT64", "123123213123", true},
		{"TYPE_UINT64", "-123123213123", false},
		{"TYPE_INT32", "123123", true},
		{"TYPE_INT32", "4294967296", false},
		{"TYPE_INT32", "-123123", true},
		{"TYPE_UINT32", "123123", true},
		{"TYPE_UINT32", "4294967296", false},
		{"TYPE_UINT32", "-123123", false},
	}
	for ii := range tests {
		if found := checkVal(tests[ii].kind, tests[ii].value); found != tests[ii].expSuccess {
			t.Fatalf("Expected %v for kind %v, val %v, found: %v", tests[ii].expSuccess, tests[ii].kind, tests[ii].value, found)
		}
	}
}
