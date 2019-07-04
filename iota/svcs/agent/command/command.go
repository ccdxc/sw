package command

// CmdCtx has all information associated to return of command
type CmdCtx struct {
	Done     bool
	Stdout   string
	TimedOut bool
	Stderr   string
	ExitCode int32
	status   chan error
}

//Wait for context
func (ctx *CmdCtx) Wait() {
	if ctx.Done {
		return
	}
	<-ctx.status
}

//Complete comlete comand
func (ctx *CmdCtx) Complete(err error) {
	ctx.status <- err
}

// CmdInfo has all information and handle to cancel later
type CmdInfo struct {
	Ctx    *CmdCtx
	Handle interface{}
}
