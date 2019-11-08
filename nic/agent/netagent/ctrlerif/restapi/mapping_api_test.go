package restapi

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"math"
	"net/http"
	"testing"

	"github.com/pensando/sw/nic/agent/protos/netproto"
	"github.com/pensando/sw/venice/utils/netutils"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestInterfaceGetsByID(t *testing.T) {
	t.Parallel()
	var intf netproto.Interface
	getURL := fmt.Sprintf("http://localhost:1337/api/mapping/interfaces/%d", 1)
	err := netutils.HTTPGet(getURL, &intf)
	AssertOk(t, err, "Interface gets must work")
	AssertEquals(t, uint64(1), intf.Status.InterfaceID, "Interface id must match")
}

func TestInterfaceGetsByNonExistingID(t *testing.T) {
	t.Parallel()
	var resp Response
	getURL := fmt.Sprintf("http://localhost:1337/api/mapping/interfaces/%d", math.MaxInt64)
	r, err := http.Get(getURL)
	AssertOk(t, err, "GETs must have a valid body")
	defer r.Body.Close()
	body, err := ioutil.ReadAll(r.Body)

	err = json.Unmarshal(body, &resp)
	AssertOk(t, err, "Unmarshaling must not fail")
	AssertEquals(t, http.StatusNotFound, resp.StatusCode, "Must respond with 404")
}

func TestInterfaceGetsByInvalidID(t *testing.T) {
	t.Parallel()
	var resp Response
	getURL := fmt.Sprintf("http://localhost:1337/api/mapping/interfaces/%f", math.MaxFloat64)
	r, err := http.Get(getURL)
	AssertOk(t, err, "GETs must have a valid body")
	defer r.Body.Close()
	body, err := ioutil.ReadAll(r.Body)

	err = json.Unmarshal(body, &resp)
	AssertOk(t, err, "Unmarshaling must not fail")
	AssertEquals(t, http.StatusBadRequest, resp.StatusCode, "Must respond with 400")
}
