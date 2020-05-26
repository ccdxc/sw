FROM registry.test.pensando.io:5000/pens-base-2:v0.6
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/community >> /etc/apk/repositories
RUN apk update
RUN apk add chrony
RUN echo http://dl-cdn.alpinelinux.org/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://dl-cdn.alpinelinux.org/alpine/v3.10/community >> /etc/apk/repositories
COPY reload-chrony-sources.sh /usr/bin
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
CMD /usr/sbin/chronyd -f /etc/pensando/pen-ntp/chrony.conf -d -s
