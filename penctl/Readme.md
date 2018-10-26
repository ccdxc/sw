# Steps to building PenCtl for linux

1. Checkout workspace
2. From a supported container/bm/etc:  
2.1. cd /sw/nic ; make penctl.gobin  
2.2. Binary will be located at /sw/nic/build/x86_64/iris/bin/penctl  

# Steps to building PenCtl for FreeBSD

1. Checkout workspace
2. From a supported container/bm/etc:  
2.1. go get github.com/mitchellh/gox  
2.1. cd /usr/src/github.com/pensando/sw/ ; gox -osarch="freebsd/amd64" github.com/pensando/sw/penctl  
2.2. Binary will be located at /usr/src/github.com/pensando/sw/penctl_freebsd_amd64  

# How to use command completion on the host?

1. Run the penctl binary once.
2. It should create a file penctl.sh in the same folder from where step #1 was done
3. Source the file in your .cshrc or .bashrc file
4. Load the new .cshrc or .bashrc file
5. Use penctl and use tab to help you with auto-completion of commands

# How to install man pages for CentOS?

1. yum install -y man-pages man
2. cp /<workspace>/penctl/docs/man/* /usr/local/share/man/man3/
3. mandb
4. man penctl

# How to setup PenCtl for the first time?

1. PenCtl uses tun interface to talk to Naples
2. Launch Memtun (memtun 1.0.0.1 &) 
3. Setup the route (ifconfig tun0 1.0.0.1 1.0.0.2)
4. Set environment variable PENETHDEV and point it to the tun interface created in step 2 or use -e/--interface flags and provide the tun interface while executing each CLI
