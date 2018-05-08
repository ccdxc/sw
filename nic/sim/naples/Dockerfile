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
    gdb \
&&  yum clean all

RUN ln -s /usr/bin/python3.6 /usr/bin/python3
RUN ln -s /usr/bin/pip3.6 /usr/bin/pip3
RUN pip3 install click click_repl

ADD nic.tgz /naples/
COPY bashrc_sim /root/.bashrc
COPY SF-Kingdom1.postman_collection.json /naples/nic/data/examples/
COPY SF-Kingdom2.postman_collection.json /naples/nic/data/examples/

HEALTHCHECK --interval=2m --timeout=5s --start-period=5m --retries=3 CMD /naples/nic/tools/health-check.sh || exit 1

CMD ["/naples/nic/tools/start-naples.sh"]
