package api

// GetObjectKind returns the kind of an object.
func (t *TypeMeta) GetObjectKind() string {
	return t.Kind
}

// GetObjectAPIVersion returns the version of an object.
func (t *TypeMeta) GetObjectAPIVersion() string {
	return t.APIVersion
}

// GetObjectMeta returns the ObjectMeta of an object.
func (o *ObjectMeta) GetObjectMeta() *ObjectMeta {
	return o
}

// GetListMeta returns the ListMeta of a list object.
func (l *ListMeta) GetListMeta() *ListMeta {
	return l
}
