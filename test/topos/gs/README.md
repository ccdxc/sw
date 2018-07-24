
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

- Run the following container, it will untar some files in the current directory
```
docker run -it -v `pwd`:/import pensando/gs-install:v0.1
```

- After executing the above command, it should look like
```
[Mac/Linux] $ ls
Vagrantfile		testbed.json		venice-conf.json	postman_collection.json
... <other postman files>
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
[vagrant@venice venice]$ docker ps
CONTAINER ID        IMAGE                                           COMMAND             CREATED             STATUS              PORTS                                                                      NAMES
397df2dcc61d        registry.test.pensando.io:5000/pens-e2e:v0.3    "/bin/sh"           3 hours ago         Up 3 hours          0.0.0.0:10000->9000/tcp, 0.0.0.0:10200->9200/tcp                           node0
0fd767acb784        registry.test.pensando.io:5000/pens-dind:v0.3   "/usr/sbin/init"    3 hours ago         Up 3 hours          0.0.0.0:8080->8080/tcp, 0.0.0.0:10001->9000/tcp, 0.0.0.0:10201->9200/tcp   node1
[vagrant@venice venice]$ docker exec -it node1 /bin/bash
```
- Examining Services from simulator VM
```
[vagrant@venice venice]$ kubectl get pods
kubectl get pods
NAME                            READY     STATUS    RESTARTS   AGE
pen-apigw-dk7dh                 1/1       Running   0          3h
pen-apiserver-788690896-lpkqn   1/1       Running   0          3h
[vagrant@venice venice]$ kubectl describe pod pen-apigw-dk7dh
...
```

### Configuring Venice

- REST Endpoints: Venice REST APIs can be accessed on port 10001
- API documentation and samples: Turin VM exposes following URL on the host where it started: `http://localhost:10001/docs`
- Sample configuration: provided in various postman files `postman_collection.json`
- All APIs can be curled from outside Turin VM on the URL paths shown by the API Browser
- Utilities like `newman` and `curl` are included in the VM e.g.
```
[vagrant@venice venice]$ newman run /vagrant/postman_collection.json -e /vagrant/postman_env.json
[vagrant@venice venice]$ curl http://localhost:10001/configs/cluster/v1/cluster
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

### Advanced
- Changing testbed configuration or enabling services. This can be done by modifying the testbed.json and venice-conf.json and execute following commands from within the VM
```
[vagrant@venice venice]$ pwd
/home/vagrant/venice
[vagrant@venice venice]$ docker rm -f $(docker ps -aq)
[vagrant@venice venice]$ ./dind/do.py -configFile testbed.json -custom_config_file /vagrant/venice-conf.json -venice_image_dir /home/vagrant/venice/venice
```
- Service logs can be found at `/var/log/pensando/...`
