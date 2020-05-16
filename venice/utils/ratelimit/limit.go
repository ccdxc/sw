package ratelimit

import (
	"io"
	"time"

	"github.com/pensando/sw/venice/utils/log"
)

type reader struct {
	r        io.Reader
	bufferSz int
	delay    time.Duration
}

// NewReader returns a reader that is rate limited by
// the given size.
func NewReader(r io.Reader, size int, delay time.Duration) io.Reader {
	return &reader{
		r:        r,
		bufferSz: size,
		delay:    delay,
	}
}

// Min returns the smaller of x or y.
func Min(x, y int) int {
	if x > y {
		return y
	}
	return x
}
func (r *reader) Read(buf []byte) (int, error) {

	minlen := Min(cap(buf), r.bufferSz)
	tmpBuf := make([]byte, minlen)
	n, err := r.r.Read(tmpBuf)
	if n <= 0 {
		log.Infof("Read with len %d err %v\n", n, err)
		return n, err
	}
	copy(buf, tmpBuf)
	log.Infof("Read %d bytes, delaying %v\n", n, r.delay)
	time.Sleep(r.delay)
	return n, err
}
