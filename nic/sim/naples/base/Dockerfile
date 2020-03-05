FROM     centos:7

LABEL maintainer="Pensando Systems Inc"

RUN yum clean all && rm -rf /var/cache/yum/* && yum update -y &&yum install -y \
    iproute \
    net-tools \
    vim \
    bzip2 \
    libtool-ltdl \
    jq \
    openssh-server \
    tar \
    https://centos7.iuscommunity.org/ius-release.rpm && \
    yum install -y \
    python36u \
    python36u-pip \
    gdb && \
    ln -sf /usr/bin/python3.6 /usr/bin/python3 && \
    ln -sf /usr/bin/pip3.6 /usr/bin/pip3 && \
    yum clean all && pip3 install click click_repl
