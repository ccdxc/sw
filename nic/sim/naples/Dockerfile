FROM     centos:7

MAINTAINER Pensando

RUN yum install -y \
    iproute \
    vim \
    bzip2 \
    tar \
    https://centos7.iuscommunity.org/ius-release.rpm

RUN yum install -y \
    python36u \
    python36u-pip \
&&  yum clean all
