package cache

import (
	"context"
	"fmt"
	"strconv"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/runtime"
)

func TestCachePagination(t *testing.T) {

	ctx := context.TODO()
	objsList := []runtime.Object{}
	filteredList := []runtime.Object{}

	for i := 0; i < 40; i++ {
		b := &testObj{}
		b.ObjectMeta.Name = "obj" + strconv.Itoa(i)
		objsList = append(objsList, b)

	}

	// Fetch first 20 results
	exp := objsList[0:20]
	opts := api.ListWatchOptions{From: 1, MaxResults: 20}
	s, e, err := processPaginationOptions(ctx, objsList, opts)
	if err != nil {
		t.Errorf("Error %s", err.Error())
	}

	filteredList = objsList[s:e]
	eq, err := checkIfObjListsEqual(exp, filteredList)
	if (eq != true) || (err != nil) {
		t.Errorf("Error %s", err.Error())
	}

	// Fetch 20 to 30 results
	exp = objsList[20:30]
	opts = api.ListWatchOptions{From: 21, MaxResults: 10}
	s, e, err = processPaginationOptions(ctx, objsList, opts)
	if err != nil {
		t.Errorf("Error %s", err.Error())
	}
	filteredList = objsList[s:e]
	eq, err = checkIfObjListsEqual(exp, filteredList)
	if (eq != true) || (err != nil) {
		t.Errorf("Error %s", err.Error())
	}

	// Negative Testcase: set MaxResults to a number more than what is supported
	opts = api.ListWatchOptions{From: 1, MaxResults: 20000}
	_, _, err = processPaginationOptions(ctx, objsList, opts)
	if err == nil {
		t.Errorf("Error, expected NumResults Failure!")
	}

	// Negative Testcase: set From more than the number of results that are present
	opts = api.ListWatchOptions{From: 50, MaxResults: 20}
	_, _, err = processPaginationOptions(ctx, objsList, opts)
	if err == nil {
		t.Errorf("Error, expected FromIndex Failure!")
	}

	// Last page case, num results returned less than MaxResults
	exp = objsList[30:]
	opts = api.ListWatchOptions{From: 31, MaxResults: 15}
	s, e, err = processPaginationOptions(ctx, objsList, opts)
	if err != nil {
		t.Errorf("Error %s", err.Error())
	}
	filteredList = objsList[s:e]
	eq, err = checkIfObjListsEqual(exp, filteredList)
	if (eq != true) || (err != nil) {
		t.Errorf("Error %s", err.Error())
	}

	t.Logf("Pass!")
}

// Given two lists of runtime.Object types (equal lengths), verfies to check if they are equal
// For testRuntimeObj type, put the name value in ObjectKind field and use it check if they are equal
func checkIfObjListsEqual(a, b []runtime.Object) (bool, error) {

	if len(a) != len(b) {
		return false, fmt.Errorf("Expecting %d objects, got %d", len(a), len(b))
	}

	// len of a and b are equal
	l := len(a)

	for i := 0; i < l; i++ {
		if a[i].GetObjectKind() != b[i].GetObjectKind() {
			return false, fmt.Errorf("Objects don't match! %+v, %+v, i %d", a[i], b[i], i)
		}
	}
	return true, nil
}
