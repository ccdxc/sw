FROM alpine:3.6
MAINTAINER "Suresh Nalluru" <suresh@pensando.io>
RUN apk update
RUN apk add libpcap-dev
COPY n4sagent /
CMD /n4sagent -npm pen-npm -resolver-urls pen-master:9002 -hostif s0 -uplink t0
