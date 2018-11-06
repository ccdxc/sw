default: penctl

penctl:
	go build -o ../nic/build/x86_64/iris/bin/penctl.linux github.com/pensando/sw/penctl
	GOOS=freebsd go build -o ../nic/build/x86_64/iris/bin/penctl.freebsd github.com/pensando/sw/penctl

clean:
	rm -rf $(GOPATH)/src/github.com/pensando/sw/penctl/bin/
