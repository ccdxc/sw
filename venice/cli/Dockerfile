# Build: MUST be executed from top level source as "docker build -t venice -f venice/cli/Dockerfile ."
# Run: "docker run -it --rm --hostname venice --name venice venice"
FROM centos/systemd

# copy some utils
RUN yum install -y wget
RUN wget -q https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64 && chmod +x jq-linux64 && mv jq-linux64 /usr/local/bin; 

# copy pensando binaries
COPY ./bin/testserver /usr/local/bin/testserver
COPY ./bin/venice /usr/local/bin/venice
COPY ./venice/cli/samples ~/samples
COPY ./venice/cli/bash_autocomplete /etc/bash_completion.d/venice

# run pensando server and give a bash shell
RUN echo "#!/bin/bash" > /tmp/pen.sh
RUN echo "/usr/local/bin/testserver &> /tmp/testserver.log &" >> /tmp/pen.sh
RUN echo "source /etc/bash_completion.d/venice" >> ~/.bashrc
RUN echo "exec /bin/bash" >> /tmp/pen.sh
RUN chmod a+x /tmp/pen.sh

WORKDIR ~/
CMD /tmp/pen.sh
