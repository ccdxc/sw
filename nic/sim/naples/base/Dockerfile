FROM     centos:7

MAINTAINER Pensando

RUN yum clean all
RUN rm -rf /var/cache/yum/*
RUN yum update -y

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
    gdb \
&&  yum clean all

RUN ln -s /usr/bin/python3.6 /usr/bin/python3
RUN ln -s /usr/bin/pip3.6 /usr/bin/pip3
RUN pip3 install click click_repl
