// Handwritten ext code to supplement generate pb.go

package apitest

// Clone copies the TestObj
func (m *TestObj) Clone(into interface{}) (interface{}, error) {
	var out *TestObj
	if into == nil {
		out = &TestObj{}
	} else {
		out = into.(*TestObj)
	}
	*out = *m
	return out, nil
}

// MakeKey creates a KV store key for TestObj
func (m *TestObj) MakeKey(prefix string) string {
	return "/test/" + prefix + "/" + m.Name
}

// Clone copies the TestObj
func (m *TestObjList) Clone(into interface{}) (interface{}, error) {
	var out *TestObjList
	if into == nil {
		out = &TestObjList{}
	} else {
		out = into.(*TestObjList)
	}
	*out = *m
	return out, nil
}

// MakeKey creates a KV store key for TestObj
func (m *TestObjList) MakeKey(prefix string) string {
	return "/test/" + prefix + "/" + m.Name
}
