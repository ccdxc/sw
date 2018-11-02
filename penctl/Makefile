default: penctl

penctl:
	go install
	GOOS=freebsd go build -o $(GOPATH)/src/github.com/pensando/sw/penctl/bin/penctl github.com/pensando/sw/penctl

clean:
	rm -rf $(GOPATH)/src/github.com/pensando/sw/penctl/bin/
