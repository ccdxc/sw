package modules

import (
	"errors"
	"log"
	"strings"

	Tests "github.com/pensando/sw/test/e2e/turin/tests"
	Infra "github.com/pensando/sw/test/utils/infra"
)

//HTTP Structure
type HTTP struct {
}

//Setup function
func (t *HTTP) Setup(ctx Infra.Context) error {
	cmd := "python -m  SimpleHTTPServer 8000"
	srcEp := ctx.FindRemoteEntityByName("app1-2")
	dstEp := ctx.FindRemoteEntityByName("app2-1")
	retcode, stdout, _ := srcEp.Exec(cmd, false, true)
	log.Print(strings.Join(stdout, "\n"))
	if retcode != 0 {
		return errors.New("Http Server Start failed")
	}

	retcode, stdout, _ = dstEp.Exec(cmd, false, true)
	log.Print(strings.Join(stdout, "\n"))
	if retcode != 0 {
		return errors.New("Http Server Start failed")
	}

	return nil
}

//Run function
func (t *HTTP) Run(ctx Infra.Context) error {
	srcEp := ctx.FindRemoteEntityByName("app1-2")
	dstEp := ctx.FindRemoteEntityByName("app2-1")
	srcIP, _ := dstEp.GetIPAddress()
	dstIP, _ := dstEp.GetIPAddress()
	cmd := "wget " + dstIP + ":8000"
	retcode, stdout, _ := srcEp.Exec(cmd, false, false)
	log.Print(strings.Join(stdout, "\n"))

	if retcode != 0 {
		return errors.New("Http test failed")
	}
	cmd = "wget " + srcIP + ":8000"
	retcode, stdout, _ = dstEp.Exec(cmd, false, false)
	log.Print(strings.Join(stdout, "\n"))

	if retcode != 0 {
		return errors.New("Http test failed")
	}
	return nil
}

//Teardown function
func (t *HTTP) Teardown(Infra.Context) error {
	return nil
}

func init() {
	tm := Tests.GetTestManager()
	tm.Register(HTTP{})
}
