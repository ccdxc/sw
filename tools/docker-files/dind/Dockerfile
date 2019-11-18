FROM centos:7
MAINTAINER Suresh Kumar Nalluru <suresh@pensando.io>

ENV container docker
RUN (cd /lib/systemd/system/sysinit.target.wants/; for i in *; do [ $i == \
systemd-tmpfiles-setup.service ] || rm -f $i; done); \
rm -f /lib/systemd/system/multi-user.target.wants/*;\
rm -f /etc/systemd/system/*.wants/*;\
rm -f /lib/systemd/system/local-fs.target.wants/*; \
rm -f /lib/systemd/system/sockets.target.wants/*udev*; \
rm -f /lib/systemd/system/sockets.target.wants/*initctl*; \
rm -f /lib/systemd/system/basic.target.wants/*;\
rm -f /lib/systemd/system/anaconda.target.wants/*;

RUN curl -sSL https://get.docker.com | CHANNEL=stable bash
RUN yum install -y softhsm libtool-ltdl-devel iproute openssh-server openssh-clients make wget
RUN yum clean all
RUN rm -rf /var/cache/yum
RUN ln -s /usr/lib/systemd/system/docker.service /etc/systemd/system/multi-user.target.wants/docker.service
RUN cp /bin/true /sbin/agetty

COPY daemon.json /etc/docker/daemon.json
VOLUME [ "/var/lib/docker" ]
VOLUME [ "/sys/fs/cgroup" ]

CMD ["/usr/sbin/init"]
