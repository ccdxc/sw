package timerqueue

import (
	"time"
)

// pqItem is something we manage in a priority queue.
type pqItem struct {
	value Entry
	time  time.Time // The priority of the item in the queue.
	// The index is needed by update and is maintained by the heap.Interface methods.
	index int // The index of the item in the heap.
}

// PriorityQueue implements heap.Interface and holds Items.
type priorityQueue []*pqItem

// Len returns the length of the queue
func (pq priorityQueue) Len() int { return len(pq) }

// Less is the comparator for the priority queue
func (pq priorityQueue) Less(i, j int) bool {
	return pq[i].time.Before(pq[j].time)
}

// Swap swaps the given indexes
func (pq priorityQueue) Swap(i, j int) {
	pq[i], pq[j] = pq[j], pq[i]
	pq[i].index = i
	pq[j].index = j
}

// Push pushes an item onto the list
func (pq *priorityQueue) Push(x interface{}) {
	n := len(*pq)
	item := x.(*pqItem)
	item.index = n
	*pq = append(*pq, item)
}

// Pop removes an item
func (pq *priorityQueue) Pop() interface{} {
	old := *pq
	n := len(old)
	item := old[n-1]
	old[n-1] = nil  // avoid memory leak
	item.index = -1 // for safety
	*pq = old[0 : n-1]
	return item
}
