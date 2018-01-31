package runtime

import (
	"testing"
	"time"

	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testInterval = time.Millisecond
)

type testBG struct {
	BG
	running bool
	count   int
}

func (tbg *testBG) run() {
	defer func() {
		tbg.running = false
		tbg.Done()
	}()

	tbg.running = true

	for {
		select {
		case <-tbg.StopCh():
			return
		case <-time.After(testInterval):
			tbg.count++
		}
	}
}

func TestBG(t *testing.T) {
	tbg1 := &testBG{}
	tbg1.Start()
	go tbg1.run()
	AssertEventually(t, func() (bool, []interface{}) {
		return tbg1.running, nil
	}, "bg did not run", "1ms", "1s")
	AssertEventually(t, func() (bool, []interface{}) {
		return (tbg1.count > 0), nil
	}, "bg did not execute", "1ms", "1s")
	tbg1.StopSync()
	AssertEventually(t, func() (bool, []interface{}) {
		return !tbg1.running, nil
	}, "bg did not run", "1ms", "1s")
}
