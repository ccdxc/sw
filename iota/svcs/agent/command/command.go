package command

//CommandCtx has all information associated to return of command
type CommandCtx struct {
	Done     bool
	Stdout   string
	TimedOut bool
	Stderr   string
	ExitCode int32
	status   chan error
}

//CommandInfo has all information and handle to cancel later
type CommandInfo struct {
	Ctx    *CommandCtx
	Handle interface{}
}
