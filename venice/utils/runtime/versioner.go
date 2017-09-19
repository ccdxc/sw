package runtime

import (
	"strconv"
)

// ObjectVersioner implements the methods to set/retrieve resource versions in an object.
type ObjectVersioner struct {
}

// NewObjectVersioner creates a ObjectVersioner.
func NewObjectVersioner() Versioner {
	return &ObjectVersioner{}
}

// SetVersion sets the resource version in an object.
func (o *ObjectVersioner) SetVersion(obj Object, resourceVersion uint64) error {
	meta, err := GetObjectMeta(obj)
	if err != nil {
		return err
	}
	verStr := ""
	if resourceVersion != 0 {
		verStr = strconv.FormatUint(resourceVersion, 10)
	}
	meta.ResourceVersion = verStr
	return nil
}

// GetVersion gets the resource version from an object.
func (o *ObjectVersioner) GetVersion(obj Object) (uint64, error) {
	meta, err := GetObjectMeta(obj)
	if err != nil {
		return 0, err
	}

	if len(meta.ResourceVersion) == 0 {
		return 0, nil
	}

	return strconv.ParseUint(meta.ResourceVersion, 10, 64)
}

// ListVersioner implements the methods to set/retrieve resource versions in an list.
type ListVersioner struct {
}

// NewListVersioner creates a ListVersioner.
func NewListVersioner() Versioner {
	return &ListVersioner{}
}

// SetVersion sets the resource version in an list.
func (l *ListVersioner) SetVersion(listObj Object, resourceVersion uint64) error {
	meta, err := GetListMeta(listObj)
	if err != nil {
		return err
	}
	verStr := ""
	if resourceVersion != 0 {
		verStr = strconv.FormatUint(resourceVersion, 10)
	}
	meta.ResourceVersion = verStr
	return nil
}

// GetVersion gets the resource version from an list.
func (l *ListVersioner) GetVersion(listObj Object) (uint64, error) {
	meta, err := GetListMeta(listObj)
	if err != nil {
		return 0, err
	}

	if len(meta.ResourceVersion) == 0 {
		return 0, nil
	}

	return strconv.ParseUint(meta.ResourceVersion, 10, 64)
}
