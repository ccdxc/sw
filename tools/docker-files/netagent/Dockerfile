FROM registry.test.pensando.io:5000/pens-base-2:v0.5
MAINTAINER "Abhi Mahagaonkar" <abhi@pensando.io>
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://package-mirror.test.pensando.io/alpine/v3.10/community >> /etc/apk/repositories
RUN apk update
RUN apk add curl libpcap-dev libc6-compat libstdc++
RUN echo http://dl-cdn.alpinelinux.org/alpine/v3.10/main > /etc/apk/repositories
RUN echo http://dl-cdn.alpinelinux.org/alpine/v3.10/community >> /etc/apk/repositories
COPY netagent /
COPY nmd /
COPY nevtsproxy /
COPY tmagent /
COPY fakedelphihub /
COPY fwgen /
CMD mkdir -p /nic/tools
COPY fwupdate /nic/tools
COPY ntsa /ntsa
COPY naples-tsa.json /naples-tsa.json
COPY VERSION /tmp
CMD /netagent -npm pen-npm -resolver-urls pen-master:9002 -hostif s0 -datapath mock -mode managed -disabletsa"
