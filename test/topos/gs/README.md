
## Instructions on starting Turin

### Prerequisites
- [Vagrant](https://releases.hashicorp.com/vagrant/2.0.2/) version 2.0.2
- [VirtualBox](https://www.virtualbox.org/wiki/Download_Old_Builds_5_2) version 5.2.12
- [Docker](https://docs.docker.com/install/) version 18.03.1-ce
- https://hub.docker.com account: share the public handle with Pensando team offline
- Minimum Required Resources for Turin VM: Cores – 4, Memory – 4G, Disk – 40G

### Package Contents
The distribution contains following files:
- Vagrantfile: describes the recipe to spin up the simulator
- testbed.json: specifies how many venice/naples nodes will need to be started
- venice-conf.json: specifies what all services need to be started
- postman_collection.json (and other postman collections): sample CRUD operations for venice features

### Getting started on a Linux/Mac machine

- Create a new directory
```
[Mac/Linux] $ mkdir pensando; cd $_
```

- Docker login to run a private container repository
```
[Mac/Linux] $ docker login
Username: <dockerhub-handle>
Password: <docker-password>
```

- Run the following container, it will untar some files in the current directory
```
docker run --rm -it -v `pwd`:/import pensando/gs-install:v0.2
```

- After executing the above command directory content should look like following
```
[Mac/Linux] $ ls
Vagrantfile		testbed.json		venice-conf.json	postman_collection.json
... <other postman files>
```

- Set the user/password in environment variables
```
[Mac/Linux] $ export DOCKER_USERNAME=<dockerhub-handle>
[Mac/Linux] $ export DOCKER_PASSWORD=<dockerhub-password>
```
Note about passwords with special bash characters: please make sure you escape
characters appropriately. For Example a password `$ecRe7&P@ssword!` need to be set as
```
[Mac/Linux] $ export DOCKER_PASSWORD='\$ecRe7\&P\@ssword\!' with single quotes
```

- Bring up Turin VM up with all functional components running
```
[Mac/Linux] $ vagrant up
... <this takes about 5-10 minutes depending on the download speeds>
```

- Environment variables can be unset if needed now
```
[Mac/Linux] $ unset DOCKER_USERNAME
[Mac/Linux] $ unset DOCKER_PASSWORD
```

#### Examining the system
- Getting into the simulator VM
```
[Mac/Linux] $ vagrant ssh venice
```
- Examining Node(s) from simulator VM
```
[vagrant@venice ~]$ docker ps
CONTAINER ID        IMAGE                                           COMMAND             CREATED             STATUS              PORTS                                                                      NAMES
5aa5eb166be0        registry.test.pensando.io:5000/pens-e2e:v0.3    "/bin/sh"           45 minutes ago      Up 45 minutes       0.0.0.0:10000->9000/tcp, 0.0.0.0:10200->9200/tcp                           node0
4d6e529b37c5        registry.test.pensando.io:5000/pens-dind:v0.3   "/usr/sbin/init"    About an hour ago   Up About an hour    0.0.0.0:8080->8080/tcp, 0.0.0.0:10001->9000/tcp, 0.0.0.0:10201->9200/tcp   node1
ebb059e60608        pen-netagent                                    "/bin/sh"           About an hour ago   Up About an hour                                                                               naples1
```
- Examining Services from simulator VM
```
[vagrant@venice ~]$ docker exec -it node0 bash
bash-4.4# kubectl get pods
NAME                            READY     STATUS    RESTARTS   AGE
pen-apigw-9zpsq                 1/1       Running   0          21s
pen-apiserver-426838463-fr2cl   1/1       Running   0          21s
pen-elastic-l2gpv               1/1       Running   0          20s
pen-evtsmgr-2hx7p               1/1       Running   0          20s
pen-evtsproxy-422j9             1/1       Running   0          21s
pen-npm-2915052316-zj8fx        1/1       Running   0          21s
pen-spyglass-367829588-d1ht4    1/1       Running   0          21s
bash-4.4# kubectl describe pod pen-apigw-9zpsq
...
```

### Using Venice

- REST Endpoints: Venice REST APIs can be accessed on port 10001
- GUI: Venice GUI is available on port 10001 
- API documentation and samples: Turin VM exposes following URL on the host where it started: `http://localhost:10001/docs`
- Sample configuration: provided in various postman files `postman_collection.json`
- All APIs can be curled from outside Turin VM on the URL paths shown by the API Browser
- Login as user `admin` and save cookie to a file
```
[vagrant@venice venice]$ curl -d '{"username":"admin", "password":"password", "tenant":"default"}' -c cookie.jar -H "Content-Type: application/json" -X POST http://localhost:10001/v1/login/
```
- Use cookie to call  REST APIs
```
[vagrant@venice venice]$ curl -b cookie.jar http://localhost:10001/configs/auth/v1/tenant/default/users
[vagrant@venice venice]$ curl -b cookie.jar http://localhost:10001/configs/cluster/v1/cluster
```
- You can also extract JWT token from cookie name `sid` and set it in `Authorization` header as `Bearer <jwt token>` while calling REST APIs
```
[vagrant@venice venice]$ curl -H "Authorization: Bearer eyJjc3JmIjoid3UwdG9kMVVGVms0cjZSWjRldnlFQW1GRS1MRHpISjFsN2R0MG9ycDlsND0iLCJleHAiOjE1MzQ4MDA2NzksImlhdCI6MTUzNDI4MjI3OSwiaXNzIjoidmVuaWNlIiwicm9sZXMiOm51bGwsInN1YiI6ImFkbWluIiwidGVuYW50IjoiZGVmYXVsdCJ9.J00lcpOdWpTZjsnyufh5U4Sh5xpEA3EKIUBgKYHQX9juSHdg1m7larOy2BTpXYAzjVHEF2zVN_NpMzo3EETOLw" http://localhost:10001/configs/auth/v1/authn-policy
```
- Utilities like `newman` and `curl` are included in the VM.
- Run postman collection
```
[vagrant@venice venice]$ newman run /vagrant/postman_collection.json -e /vagrant/postman_env.json
```

### Restarting and Destroying the VM
- Reprovision the VM and bring to a clean state using:
```
[Mac/Linux] $ vagrant reload --provision
```
- Completely destroy the VM and related files
```
[Mac/Linux] $ vagrant destroy -f
```
Delete any leftover VMs using virtualbox's vboxmanage command
```
[Mac/Linux] $ vboxmanage list vms
"packer-centos-7.4-x86_64_1532074216818_71830" {f77847b6-160f-4b62-952a-374e73a5d08e}
[Mac/Linux] $ $ vboxmanage unregistervm --delete f77847b6-160f-4b62-952a-374e73a5d08e
```

### Advanced
- Changing testbed configuration or enabling services. This can be done by modifying the testbed.json and venice-conf.json and execute following commands from within the VM
```
[vagrant@venice venice]$ pwd
/home/vagrant/venice
[vagrant@venice venice]$ docker rm -f $(docker ps -aq)
[vagrant@venice venice]$ ./dind/do.py -configFile /vagrant/testbed.json -custom_config_file /vagrant/venice-conf.json -venice_image_dir /home/vagrant/venice/venice
```
- Service logs can be found at `/var/log/pensando/...`
