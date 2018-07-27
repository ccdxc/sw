package remoteEntity

//RemoteEntity Type of RemoteEntity and interface to them
type RemoteEntity interface {
	Exec(cmd string) error
	GetEntityName() string
	Setup() error
}
