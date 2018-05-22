FROM alpine:3.6
RUN apk update
RUN apk add chrony
COPY reload-chrony-sources.sh /usr/bin
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
CMD /usr/sbin/chronyd -f /etc/pensando/ntp/chrony.conf -d -s
