package vcprobe

import (
	"reflect"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

var testSet1 = []string{
	"a1",
	"a2",
	"a3",
	"a4",
	"a5",
}

var testSet2 = []string{
	"a1",
	"a3",
	"a5",
	"a2",
	"a4",
}

var testSet3 = []string{
	"a2",
	"a4",
	"a6",
}

func TestDeltaSet(t *testing.T) {
	// verify diff with different order
	d1 := NewDeltaStrSet(testSet1)
	Assert(t, d1.Count() == len(testSet1), "Incorrect count")
	d2 := d1.Diff(testSet2)
	Assert(t, d2.Count() == 0, "Diff failed")

	d3 := d1.Diff(testSet3)

	Assert(t, len(d3.Additions()) == 1, "Expected 1 addition")
	Assert(t, len(d3.Deletions()) == 3, "Expected 3 deletions")

	// ApplyDiff
	d1.ApplyDiff(d3)
	Assert(t, d1.Count() == len(testSet3), "ApplyDiff failed")

	// Diff against an empty set
	d4 := NewDeltaStrSet([]string{})
	d5 := d4.Diff(testSet3)
	Assert(t, len(d5.Additions()) == 3, "Expected 3 additions")
	Assert(t, len(d5.Deletions()) == 0, "Expected 0 deletions")
	d4.ApplyDiff(d5)
	Assert(t, d4.Count() == len(testSet3), "ApplyDiff failed")

	// Single items
	d4.AddSingle("b1")
	Assert(t, d4.Count() == (len(testSet3)+1), "AddSingle failed")
	d4.RemoveSingle("a6")
	Assert(t, d4.Count() == len(testSet3), "RemoveSingle failed")
	d5 = d4.Diff(testSet3)
	a := d5.Additions()
	exp := []string{"a6"}
	Assert(t, reflect.DeepEqual(a, exp), "Diff failed")
	b := d5.Deletions()
	exp = []string{"b1"}
	Assert(t, reflect.DeepEqual(b, exp), "Diff failed")
	Assert(t, len(d5.Items()) == 2, "Items incorrect")

	d4.ApplyDiff(d5)
	d4.Add(testSet1)
	Assert(t, d4.Count() == 6, "Add failed")
}
