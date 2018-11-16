default: penctl

penctl:
	export GIT_COMMIT=`git rev-parse --short HEAD` && go build -ldflags "-X main.GitCommit=$$GIT_COMMIT -X main.PenCtlVer=`cat version.txt`" -o ../nic/build/x86_64/iris/bin/penctl.linux github.com/pensando/sw/penctl

	GOOS=freebsd export GIT_COMMIT=`git rev-parse --short HEAD` && go build -ldflags "-X main.GitCommit=$$GIT_COMMIT -X main.PenCtlVer=`cat version.txt`" -o ../nic/build/x86_64/iris/bin/penctl.freebsd github.com/pensando/sw/penctl

clean:
	rm -rf $(GOPATH)/src/github.com/pensando/sw/penctl/bin/
