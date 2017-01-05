
### Devloper's environment
The goal is to create an environment that is consistent and predictable for building binaries, running tests,
experimenting with the changes.

#### Prerequisits
- Git
- [Virtualbox 5.1.4] (https://www.virtualbox.org/wiki/Download_Old_Builds_5_1)
- [Vagrant 1.8.6] (https://www.vagrantup.com/downloads.html)


##### 1. Clone/Create repository
```
$ mkdir -p src/github.com/pensando/sw; cd $_
$ git clone https://github.com/pensando/sw

$ git clone https://github.com/<your-github-id>/<forked-repo>
```

##### 2. Start Development Environment
```
$ cd sw/tools; make dev
```

##### 3. SSH into the nodes, make changes, run tests
```
$ make dev-ssh

< inside the VM >

node1 $ exit
```

##### 4. Clean up
```
$ make dev-clean
```
Note: cleaning up would destroy the VMs, but it would *not* destroy the
code/binaries that were altered from within the VM
