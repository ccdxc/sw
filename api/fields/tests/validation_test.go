package tests

import (
	"fmt"
	"testing"
	"time"

	"github.com/gogo/protobuf/types"

	"github.com/pensando/sw/api/fields"
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

		// single level keys are needed for objects that do not have spec/status. e.g. events
		"abcd",
	}
	badKeys := []string{
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
		if err := fields.ValidateFieldKey(goodKeys[ii]); err != nil {
			t.Fatalf("%v", err)
		}
	}
	for ii := range badKeys {
		if err := fields.ValidateFieldKey(badKeys[ii]); err == nil {
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
		" >  ",
		" < ",
		" >= ",
		"  <= ",
		">",
		"<",
		">=",
		"<=",
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
		"< =",
		"=> ",
		"==<=",
	}
	for ii := range goodOps {
		if err := fields.ValidateFieldOp(goodOps[ii]); err != nil {
			t.Fatalf("%v", err)
		}
	}
	for ii := range badOps {
		if err := fields.ValidateFieldOp(badOps[ii]); err == nil {
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
		if err := fields.ValidateFieldVals(goodVals[ii]); err != nil {
			t.Fatalf("%v", err)
		}
	}
	for ii := range badVals {
		if err := fields.ValidateFieldVals(badVals[ii]); err == nil {
			t.Fatalf("Validation success, should have failed: %v", badVals[ii])
		}
	}
}

func TestSelectorValidation(t *testing.T) {
	timeNow, _ := types.TimestampProto(time.Now())
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
		"x.x<a",
		"x.x>1",
		"x.x>1,z<5",
		"x.x>=5",
		"x.x>5,y.y>=7",
		fmt.Sprintf("xx>=%v", timeNow.String()),
		"x.x= something dummy  z.z=d ", // TODO: this case should fail
	}
	badSels := []string{
		"",
		"x.x==",
		"x.x[*]==",
		"x.x[*]*=",
		"x.x=a||y.y=b",
		"x.x==a==b",
		"!x.x=a",
		"!x.x",
		"x.x in a",
		"x.x in (a",
		"x.x in (a,b",
		"x.x=(a,b)",
		"a.b[999.99].cd[x] in (x,z)",
		fmt.Sprintf("xx>  =%v", timeNow.String()),
	}
	for ii := range goodSels {
		if err := fields.ValidateSelector(goodSels[ii]); err != nil {
			t.Fatalf("%v", err)
		}
	}
	for ii := range badSels {
		if err := fields.ValidateSelector(badSels[ii]); err == nil {
			t.Fatalf("Validation success, should have failed: %v", badSels[ii])
		}
	}
}
