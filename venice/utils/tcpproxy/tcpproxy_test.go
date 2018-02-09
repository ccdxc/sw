package tcpproxy

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/venice/utils/testutils"
)

func TestTcpProxy(t *testing.T) {
	ts := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintln(w, "Hello")
	}))
	defer ts.Close()

	proxy := New()
	defer proxy.Stop()

	proxy.DialTimeout(time.Second)

	addr := strings.Replace(ts.URL, "http://", "", 1)
	proxy.Target(addr)

	err := proxy.Run("127.0.0.1:0")
	testutils.Assert(t, err == nil, "proxy.Run should succeed")

	proxyAddr := proxy.Addr()
	res, err := http.Get(fmt.Sprintf("http://%s", proxyAddr.String()))
	testutils.Assert(t, err == nil, "http Get should succeed")

	greeting, err := ioutil.ReadAll(res.Body)
	testutils.Assert(t, err == nil, "http body should have content")

	res.Body.Close()
	testutils.Assert(t, string(greeting) == "Hello\n", "http body should have expected content", string(greeting))
}
