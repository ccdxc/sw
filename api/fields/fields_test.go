/*
Copyright 2015 The Kubernetes Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package fields

import (
	"testing"
)

func matches(t *testing.T, fs Set, want string) {
	if fs.String() != want {
		t.Errorf("Expected '%s', but got '%s'", want, fs.String())
	}
}

func TestSetString(t *testing.T) {
	matches(t, Set{"x": "y"}, "x=y")
	matches(t, Set{"foo": "bar"}, "foo=bar")
	matches(t, Set{"foo": "bar", "baz": "qup"}, "baz=qup,foo=bar")
}

func TestFieldHas(t *testing.T) {
	fieldHasTests := []struct {
		Fs  Fields
		Key string
		Has bool
	}{
		{Set{"x": "y"}, "x", true},
		{Set{"x": ""}, "x", true},
		{Set{"x": "y"}, "foo", false},
	}
	for _, lh := range fieldHasTests {
		if has := lh.Fs.Has(lh.Key); has != lh.Has {
			t.Errorf("%#v.Has(%#v) => %v, expected %v", lh.Fs, lh.Key, has, lh.Has)
		}
	}
}

func TestFieldGet(t *testing.T) {
	fs := Set{"x": "y"}
	if fs.Get("x") != "y" {
		t.Errorf("Set.Get is broken")
	}
	if fs.Get("y") != "" {
		t.Errorf("Set.Get is broken")
	}
}
