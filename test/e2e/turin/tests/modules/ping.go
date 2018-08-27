package modules

import (
	"errors"
	"log"
	"strings"

	Tests "github.com/pensando/sw/test/e2e/turin/tests"
	Infra "github.com/pensando/sw/test/utils/infra"
)

//PING Structure
type PING struct {
}

//Setup Function
func (t *PING) Setup(ctx Infra.Context) error {
	return nil
}

//Run Function
func (t *PING) Run(ctx Infra.Context) error {
	srcEp := ctx.FindRemoteEntityByName("app1-2")
	dstEp := ctx.FindRemoteEntityByName("app2-1")
	srcIP, _ := srcEp.GetIPAddress()
	dstIP, _ := dstEp.GetIPAddress()
	cmd := "ping -c 5 " + srcIP
	retcode, stdout, _ := dstEp.Exec(cmd, false, false)
	log.Print(strings.Join(stdout, "\n"))

	if retcode != 0 {
		return errors.New("Ping test failed")
	}
	cmd = "ping -c 5 " + dstIP
	retcode, stdout, _ = srcEp.Exec(cmd, false, false)
	log.Print(strings.Join(stdout, "\n"))

	if retcode != 0 {
		return errors.New("Ping test failed")
	}
	return nil
}

//Teardown function
func (t *PING) Teardown(ctx Infra.Context) error {
	return nil
}

func init() {
	tm := Tests.GetTestManager()
	tm.Register(PING{})
}
