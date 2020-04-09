default: penctl

penctl:
	tools/penctl_version.sh
	export GIT_COMMIT=`git rev-parse --short HEAD` && CGO_ENABLED=0  go build -ldflags "-X main.GitCommit=$$GIT_COMMIT -X main.PenCtlVer=`cat version.txt` -X main.BuiltTime=$$(date +%Y.%m.%d-%H:%M:%S)" -o ../nic/build/x86_64/iris/capri/bin/penctl.linux github.com/pensando/sw/penctl
	GOOS=freebsd export GIT_COMMIT=`git rev-parse --short HEAD` && CGO_ENABLED=0  go build -ldflags "-X main.GitCommit=$$GIT_COMMIT -X main.PenCtlVer=`cat version.txt` -X main.BuiltTime=$$(date +%Y.%m.%d-%H:%M:%S)" -o ../nic/build/x86_64/iris/capri/bin/penctl.freebsd github.com/pensando/sw/penctl
	GOOS=windows export GIT_COMMIT=`git rev-parse --short HEAD` && CGO_ENABLED=0  go build -ldflags "-X main.GitCommit=$$GIT_COMMIT -X main.PenCtlVer=`cat version.txt` -X main.BuiltTime=$$(date +%Y.%m.%d-%H:%M:%S)" -o ../nic/build/x86_64/iris/capri/bin/penctl.win64.exe github.com/pensando/sw/penctl
	GOOS=windows GOARCH=386 export GIT_COMMIT=`git rev-parse --short HEAD` && CGO_ENABLED=0  go build -ldflags "-X main.GitCommit=$$GIT_COMMIT -X main.PenCtlVer=`cat version.txt` -X main.BuiltTime=$$(date +%Y.%m.%d-%H:%M:%S)" -o ../nic/build/x86_64/iris/capri/bin/penctl.win32.exe github.com/pensando/sw/penctl

clean:
	rm -rf $(GOPATH)/src/github.com/pensando/sw/penctl/bin/
