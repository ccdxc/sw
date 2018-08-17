package safelist

import (
	"fmt"
	"testing"

	tu "github.com/pensando/sw/venice/utils/testutils"
)

func TestInsert(t *testing.T) {
	l := New()
	if l == nil {
		t.Fatalf("create List failed")
	}
	x := 100
	e := l.Insert(x)
	if l.Len() != 1 {
		t.Errorf("wrong length expecting [1] got [%d]", l.Len())
	}

	x1 := l.Remove(e)
	if x1.(int) != x {
		t.Errorf("wrong value expecting [%d] got [%d]", x, x1.(int))
	}
	if l.Len() != 0 {
		t.Errorf("wrong length expecting [1] got [%d]", l.Len())
	}

	x = 200
	l.Insert(x)
	x = 201
	l.Insert(x)
	x = 202
	l.Insert(x)

	if l.Len() != 3 {
		t.Errorf("wrong length expecting [1] got [%d]", l.Len())
	}

	verifyCount := 0
	verifyValues := []int{200, 201, 202}
	iterfunc := func(i interface{}) bool {
		d := i.(int)
		found := false
		for _, v := range verifyValues {
			if v == d {
				found = true
				verifyCount++
				break
			}
		}
		if !found {
			t.Errorf("did not find value %d", d)
		}
		return true
	}
	remfunc := func(i interface{}) {
		iterfunc(i)
	}

	l.Iterate(iterfunc)
	if verifyCount != 3 {
		t.Errorf("not all elements found expecting [3] found [%d", verifyCount)
	}

	if l.Len() != 3 {
		t.Errorf("wrong length expecting [3] got [%d]", l.Len())
	}

	verifyCount = 0
	l.RemoveAll(remfunc)
	if verifyCount != 3 {
		t.Errorf("not all elements found expecting [3] found [%d", verifyCount)
	}
	if l.Len() != 0 {
		t.Errorf("wrong length expecting [1] got [%d]", l.Len())
	}

	x = 200
	l.Insert(x)
	x = 201
	l.Insert(x)
	x = 202
	l.Insert(x)
	if l.Len() != 3 {
		t.Errorf("wrong length expecting [3] got [%d]", l.Len())
	}
	rtillfn := func(len int, i interface{}) bool {
		d := i.(int)
		if d > 201 {
			return false
		}
		return true
	}
	status := l.RemoveTill(rtillfn)
	tu.Assert(t, status == false, fmt.Sprintf("failed to return error status, got %v", status))
	if l.Len() != 1 {
		t.Errorf("wrong length expecting [1] got [%d]", l.Len())
	}

	status = l.RemoveTill(func(len int, in interface{}) bool {
		return true
	})
	tu.Assert(t, status == true, fmt.Sprintf("failed to return status, got %v", status))
	tu.Assert(t, l.Len() == 0, fmt.Sprintf("list is not empty, got %v", l.Len()))
}
