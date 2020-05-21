package timerqueue

import (
	"context"
	"testing"
	"time"

	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestTimerQueue(t *testing.T) {
	res := make(chan int, 10)
	q := NewQueue()
	ctx, cancel := context.WithCancel(context.Background())
	go func() {
		q.Run(ctx)
	}()
	addItem := func(i int) {
		res <- i
	}
	q.Add(func() {
		addItem(2)
	}, 50*time.Millisecond)
	q.Add(func() {
		addItem(0)
	}, 20*time.Millisecond)
	q.Add(func() {
		addItem(1)
	}, 30*time.Millisecond)
	q.Add(func() {
		addItem(3)
		// Ensure item can add itself onto the queue again
		q.Add(func() {
			addItem(4)
		}, 10*time.Millisecond)
	}, 100*time.Millisecond)

	resArr := []int{}
	for len(resArr) < 5 {
		select {
		case val := <-res:
			resArr = append(resArr, val)
		case <-time.After(2 * time.Second):
			t.Fatalf("Failed to receive all messages")
		}
	}
	cancel()
	AssertEquals(t, []int{0, 1, 2, 3, 4}, resArr, "Values did not match")

}
