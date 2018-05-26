FROM node:9.11.1
MAINTAINER "Ravi Gadde" <ravi@pensando.io>

RUN echo 'deb http://dl.google.com/linux/chrome/deb/ stable main' >> /etc/apt/sources.list
RUN wget https://dl-ssl.google.com/linux/linux_signing_key.pub
RUN apt-key add linux_signing_key.pub
RUN apt-get update
RUN apt-get install curl sudo google-chrome-stable  -y
RUN ln /usr/bin/google-chrome-stable /usr/bin/chrome
RUN chmod 755 /usr/bin/chrome

RUN curl -SsLO https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64 && chmod +x jq-linux64 && mv jq-linux64 /usr/local/bin ;

# Install npm packages
RUN mkdir -p /usr/local/lib/web-app-framework
COPY web-app-framework/package.json /usr/local/lib/web-app-framework
COPY web-app-framework/package-lock.json /usr/local/lib/web-app-framework
RUN cd /usr/local/lib/web-app-framework && npm install phantomjs-prebuilt && npm install
RUN cd /usr/local/lib/web-app-framework && tar zcvf node_modules.tgz node_modules

RUN mkdir -p /usr/local/lib/webapp
COPY webapp/package.json /usr/local/lib/webapp
COPY webapp/package-lock.json /usr/local/lib/webapp
RUN cd /usr/local/lib/webapp && npm install phantomjs-prebuilt
RUN cd /usr/local/lib/webapp && npm install && tar zcvf node_modules.tgz node_modules

# Install ng
USER node
RUN mkdir /home/node/.npm-global
ENV PATH=/home/node/.npm-global/bin:$PATH
ENV NPM_CONFIG_PREFIX=/home/node/.npm-global
RUN npm install -g @angular/cli@1.7.3
RUN ng -v

USER root
RUN chmod -R 777 /root
WORKDIR /import/src/github.com/pensando/sw
ENV PATH=/usr/local/go/bin:/opt/bin:/import/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/home/node/.npm-global/bin:/usr/local/lib/web-app-framework/node_modules/.bin
ENV HOME=/root
ENV NPM_CONFIG_PREFIX=/root/.npm
CMD make ui
