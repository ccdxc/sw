package safelist

import (
	"container/list"
	"sync"
)

// SafeList is a concurency safe list. Only methods implemented here are thread safe
type SafeList struct {
	sync.Mutex
	*list.List
}

// New create a new SafeList
func New() *SafeList {
	return &SafeList{List: list.New()}
}

// Insert inserts the element into the list
func (l *SafeList) Insert(e interface{}) interface{} {
	l.Lock()
	defer l.Unlock()
	return l.List.PushBack(e)
}

// Remove removes an element from the list
func (l *SafeList) Remove(e interface{}) interface{} {
	l.Lock()
	defer l.Unlock()
	el, ok := e.(*list.Element)
	if ok {
		return l.List.Remove(el)
	}
	return nil
}

// Len returns the len of the list
func (l *SafeList) Len() int {
	l.Lock()
	defer l.Unlock()
	return l.List.Len()
}

// RemoveAll deletes all elements in the list after calling fn for each element
func (l *SafeList) RemoveAll(fn func(interface{})) {
	l.Lock()
	defer l.Unlock()
	el := l.Front()
	for el != nil {
		fn(el.Value)
		l.List.Remove(el)
		el = l.Front()
	}
}

// Iterate walks all elements in the list calling fn for each element
func (l *SafeList) Iterate(fn func(interface{})) {
	l.Lock()
	defer l.Unlock()
	el := l.Front()
	for el != nil {
		fn(el.Value)
		el = el.Next()
	}
}
