FROM "golang:1.13.4"

# enable jessie-backports
#RUN echo "deb http://http.debian.net/debian jessie-backports main" > /etc/apt/sources.list.d/jessie-backports.list

# install required utils
RUN apt-get update && apt-get install rsync libpcap-dev -y && \
  apt-get install net-tools && \
  apt-get install -y libltdl-dev softhsm2 unzip

# install protoc
RUN curl -LO https://github.com/protocolbuffers/protobuf/releases/download/v3.5.1/protoc-3.5.1-linux-x86_64.zip && \
  unzip protoc-3.5.1-linux-x86_64.zip -d tmp && \
  mv tmp/bin/protoc /usr/local/bin && \
  mv tmp/include/google /usr/local/include/ && \
  rm -rf protoc-3.5.1-linux-x86_64.zip tmp

# install docker
RUN curl -sSL https://get.docker.com | CHANNEL=stable bash
COPY daemon.json /etc/docker/daemon.json

CMD /bin/bash
