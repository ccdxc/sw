package runtime

import (
	"io"
)

// StringsReader implements io.ReadCloser
type StringsReader struct {
	strs       []string // strings added to the reader
	strCursor  int      // points at the string being read
	charCursor int      // points at the char being read
}

func smallerOf(x, y int) int {
	if x < y {
		return x
	}

	return y
}

// NewStringsReader makes a StringsReader from ss
func NewStringsReader(ss []string) *StringsReader {
	return &StringsReader{strs: ss}
}

// AddString adds a string to the reader
func (sr *StringsReader) AddString(s string) {
	sr.strs = append(sr.strs, s)
}

// Strings returns the slice of strings
func (sr *StringsReader) Strings() []string {
	return sr.strs
}

// Read implements the io.Reader interface
func (sr *StringsReader) Read(p []byte) (n int, err error) {
	bufLeft := len(p)
	offOut := 0

	readStr := func(s string, offIn int) int {
		strLen := len(s) - offIn
		toRead := smallerOf(strLen, bufLeft)
		copy(p[offOut:(offOut+toRead)], s[offIn:(offIn+toRead)])
		offOut += toRead
		bufLeft -= toRead
		return offIn + toRead
	}

	oi := sr.strCursor
	oj := sr.charCursor
	defer func() {
		sr.charCursor = oj
		sr.strCursor = oi
	}()
	for bufLeft > 0 && oi < len(sr.strs) {
		oj = readStr(sr.strs[oi], oj)
		if oj < len(sr.strs[oi]) {
			return offOut, nil
		}

		oi++
		oj = 0
	}

	if bufLeft == 0 {
		return offOut, nil
	}

	return offOut, io.EOF
}

// Close implements the io.Closer interface
func (sr *StringsReader) Close() error {
	return nil
}

// Size returns the length of the reader contents
func (sr *StringsReader) Size() int64 {
	size := 0

	for _, s := range sr.strs {
		size += len(s)
	}

	return int64(size)
}
