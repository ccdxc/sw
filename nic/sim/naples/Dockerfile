FROM     centos:7

MAINTAINER Pensando

RUN yum install -y \
    iproute \
    net-tools \
    vim \
    bzip2 \
    tar \
    https://centos7.iuscommunity.org/ius-release.rpm

RUN yum install -y \
    python36u \
    python36u-pip \
&&  yum clean all

ADD nic.tgz /naples/

CMD ["/naples/nic/tools/start-naples.sh"]
