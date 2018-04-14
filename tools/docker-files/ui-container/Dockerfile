FROM node:9.11.1
MAINTAINER "Ravi Gadde" <ravi@pensando.io>
RUN apt-get update
RUN apt-get install curl sudo -y

RUN curl -SsLO https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64 && chmod +x jq-linux64 && mv jq-linux64 /usr/local/bin ;

# Install npm packages
RUN mkdir -p /usr/local/lib/web-app-framework
COPY web-app-framework/package.json /usr/local/lib/web-app-framework
COPY web-app-framework/package-lock.json /usr/local/lib/web-app-framework
RUN cd /usr/local/lib/web-app-framework && npm install

RUN mkdir -p /usr/local/lib/webapp
COPY webapp/package.json /usr/local/lib/webapp
COPY webapp/package-lock.json /usr/local/lib/webapp
RUN cd /usr/local/lib/webapp && npm install

# Install ng
USER node
RUN mkdir /home/node/.npm-global
ENV PATH=/home/node/.npm-global/bin:$PATH
ENV NPM_CONFIG_PREFIX=/home/node/.npm-global
RUN npm install -g @angular/cli@1.7.3 
RUN ng -v

USER root
WORKDIR /import/src/github.com/pensando/sw
ENV PATH=/usr/local/go/bin:/opt/bin:/import/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/home/node/.npm-global/bin:/usr/local/lib/web-app-framework/node_modules/.bin
CMD make ui-cached
