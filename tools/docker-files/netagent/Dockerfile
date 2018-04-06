FROM registry.test.pensando.io:5000/pens-base-2:v0.1
MAINTAINER "Abhi Mahagaonkar" <abhi@pensando.io>
RUN apk update
RUN apk add libpcap-dev
COPY netagent /
COPY nmd /
CMD /netagent -npm pen-npm -resolver-urls pen-master:9002 -hostif s0 -datapath mock -mode managed"
