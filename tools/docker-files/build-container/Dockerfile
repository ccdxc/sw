FROM centos/systemd
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>

COPY goinstall.bash /tmp/goinstall.bash

RUN yum -y install https://download.fedoraproject.org/pub/epel/7/x86_64/e/epel-release-7-9.noarch.rpm
RUN sed -i -e 's/^enabled=1/enabled=0/' /etc/yum.repos.d/epel.repo

RUN yum install -y net-tools tcpdump wget
RUN yum group install -y "Development Tools"


#COPY kubernetes.repo /etc/yum.repos.d
#RUN yum install -y docker kubelet kubeadm kubectl kubernetes-cni
#RUN sed -i -e 's/--selinux-enabled//' /etc/sysconfig/docker

RUN wget -q https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64 && chmod +x jq-linux64 && mv jq-linux64 /usr/local/bin ; 

#golang
RUN cd /tmp ; wget https://storage.googleapis.com/golang/go1.8.1.linux-amd64.tar.gz ; tar -C /usr/local -xzf go1.8.1.linux-amd64.tar.gz ; rm go1.8.1.linux-amd64.tar.gz

# Install Swagger
RUN curl -o /usr/local/bin/swagger -L'#' https://github.com/go-swagger/go-swagger/releases/download/0.10.0/swagger_linux_amd64 ; chmod +x /usr/local/bin/swagger

# extra go packages
RUN chmod +x /tmp/goinstall.bash ;  bash -c /tmp/goinstall.bash

COPY env.sh /etc/profile.d/env.sh

WORKDIR /import/src/github.com/pensando/sw
ENV GOPATH=/import
ENV PATH=/usr/local/go/bin:/opt/bin:/import/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
CMD make build
