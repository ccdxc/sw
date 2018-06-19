FROM     centos:7

LABEL maintainer="Pensando Systems Inc"

RUN yum clean all && rm -rf /var/cache/yum/* && yum update -y &&yum install -y \
    iproute \
    net-tools \
    vim \
    bzip2 \
    tar \
    https://centos7.iuscommunity.org/ius-release.rpm && \
    yum install -y \
    python36u \
    python36u-pip \
    gdb && \
    ln -s /usr/bin/python3.6 /usr/bin/python3 && \
    ln -s /usr/bin/pip3.6 /usr/bin/pip3 && \
    yum clean all && pip3 install click click_repl