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
	defer l.Unlock()
	l.Lock()
	ret := l.List.PushBack(e)
	return ret
}

// Remove removes an element from the list
func (l *SafeList) Remove(e interface{}) interface{} {
	defer l.Unlock()
	l.Lock()
	el, ok := e.(*list.Element)
	if ok {
		return l.List.Remove(el)
	}
	return nil
}

// Len returns the len of the list
func (l *SafeList) Len() int {
	defer l.Unlock()
	l.Lock()
	return l.List.Len()
}

// Front returns the front of the list
func (l *SafeList) Front() *list.Element {
	defer l.Unlock()
	l.Lock()
	return l.List.Front()
}

// Back returns the tail of the list
func (l *SafeList) Back() *list.Element {
	defer l.Unlock()
	l.Lock()
	return l.List.Back()
}

// RemoveAll deletes all elements in the list after calling fn for each element
func (l *SafeList) RemoveAll(fn func(interface{})) {
	defer l.Unlock()
	l.Lock()
	el := l.List.Front()
	for el != nil {
		fn(el.Value)
		l.List.Remove(el)
		el = l.List.Front()
	}
}

// RemoveTill deletes elements in the list till the callback function returns false
func (l *SafeList) RemoveTill(fn func(int, interface{}) bool) {
	defer l.Unlock()
	l.Lock()
	el := l.List.Front()
	for el != nil {
		n := el.Next()
		if !fn(l.List.Len(), el.Value) {
			return
		}
		l.List.Remove(el)
		el = n
	}
}

// Iterate walks all elements in the list calling fn for each element. The iterate
//  is aborted if the iter function returns false.
func (l *SafeList) Iterate(fn func(interface{}) bool) {
	defer l.Unlock()
	l.Lock()
	el := l.List.Front()
	for el != nil {
		if !fn(el.Value) {
			return
		}
		el = el.Next()
	}
}
