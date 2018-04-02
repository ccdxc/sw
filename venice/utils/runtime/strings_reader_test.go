package runtime

import (
	"bytes"
	"fmt"
	"io"
	"testing"

	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	res1 = []byte("meas1,host=host0,zone=west cpu=70i,mem=129000i 110000\nmeas1,host=host1,zone=west cpu=71i,mem=129001i 110001\nmeas1,host=host2,zone=west cpu=72i,mem=129002i 110002\nmeas1,host=host3,zone=west cpu=73i,mem=129003i 110003\n")
	res2 = []byte("Cow")
	res3 = []byte(" jumped over the moon")
)

func TestStringsReader(t *testing.T) {

	var ss []string
	for ix := 0; ix < 2; ix++ {
		ss = append(ss, fmt.Sprintf("meas1,host=host%d,zone=%s cpu=%di,mem=%di %d\n", ix, "west", 70+ix, 129000+ix, 110000+ix))
	}

	sr := NewStringsReader(ss)
	for ix := 2; ix < 4; ix++ {
		sr.AddString(fmt.Sprintf("meas1,host=host%d,zone=%s cpu=%di,mem=%di %d\n", ix, "west", 70+ix, 129000+ix, 110000+ix))
	}

	rdStrs := sr.Strings()
	Assert(t, len(rdStrs) == 4, "Expected 4 strings")

	rBuf := make([]byte, len(res1)+1)

	_, err := sr.Read(rBuf)
	Assert(t, err == io.EOF, "Expected EOF")
	Assert(t, bytes.Compare(rBuf[0:len(res1)], res1) == 0, "Expected "+string(res1)+"got "+string(rBuf))
	ss = []string{string(res2) + string(res3)}
	sr = NewStringsReader(ss)
	rBuf = make([]byte, len(res2))
	_, err = sr.Read(rBuf)
	Assert(t, err == nil, "Expected no error")
	Assert(t, bytes.Compare(rBuf, res2) == 0, "Expected "+string(res2)+"got "+string(rBuf))
	rBuf = make([]byte, len(res3))
	_, err = sr.Read(rBuf)
	Assert(t, (err == nil) || (err == io.EOF), "Expected EOF")
	Assert(t, bytes.Compare(rBuf, res3) == 0, "Expected "+string(res3)+"got "+string(rBuf))
}
