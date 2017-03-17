FROM centos/systemd
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
RUN yum -y install ntp; yum clean all; systemctl enable ntpd;
COPY target /
EXPOSE 80
CMD ["/usr/sbin/init"]
