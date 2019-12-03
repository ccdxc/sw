FROM node:10.15.3-stretch
MAINTAINER "Rohan Sikdar" <rsikdar@pensando.io>

# Install ng
USER node
RUN mkdir /home/node/.npm-global
ENV PATH=/home/node/.npm-global/bin:$PATH
ENV NPM_CONFIG_PREFIX=/home/node/.npm-global
RUN npm install -g @angular/cli@7.0.4
# Install spectacle-docs for venice docs generation
RUN npm install -g spectacle-docs
RUN ng -v

USER root
RUN chmod -R 777 /root

ENV PATH=/home/node/.npm-global/bin:$PATH

RUN mkdir -p /usr/local/lib/web-app-framework /usr/local/lib/webapp /usr/local/lib/venice-sdk /usr/local/lib/swagger-gen /home/node/.npm-global /root/.npm && chmod -R 777 /root
RUN echo 'deb http://dl.google.com/linux/chrome/deb/ stable main' >> /etc/apt/sources.list
RUN wget https://dl-ssl.google.com/linux/linux_signing_key.pub
RUN apt-key add linux_signing_key.pub
RUN apt-get update
RUN apt-get install -y vim
RUN apt-get install curl sudo google-chrome-stable  -y
RUN ln /usr/bin/google-chrome-stable /usr/bin/chrome
RUN chmod 755 /usr/bin/chrome

RUN npm i -g yarn@1.16.0

# setting offline mirror location
RUN yarn config set yarn-offline-mirror /root/node-packages-offline-cache

# Copying package.json files
COPY venice-sdk/package.json /usr/local/lib/venice-sdk
COPY web-app-framework/package.json /usr/local/lib/web-app-framework
COPY webapp/package.json /usr/local/lib/webapp

# Copying yarn.lock files
COPY venice-sdk/yarn.lock /usr/local/lib/venice-sdk
COPY web-app-framework/yarn.lock /usr/local/lib/web-app-framework
COPY webapp/yarn.lock /usr/local/lib/webapp

# Copy and extract ts generator so that its node modules will be stored in the offline mirror
COPY venice-sdk/pensando-swagger-ts-generator-1.1.29.tgz /usr/local/lib/swagger-gen
RUN cd /usr/local/lib/swagger-gen && yarn add file:pensando-swagger-ts-generator-1.1.29.tgz
RUN rm -rf /usr/local/lib/swagger-gen 

# yarn install packages to populate node_modules and offline mirror 
# create tar and then remove node modules
RUN cd /usr/local/lib/venice-sdk && yarn install --frozen-lockfile
RUN cd /usr/local/lib/venice-sdk && tar zcf node_modules.tgz node_modules
RUN rm -rf /usr/local/lib/venice-sdk/node_modules

RUN cd /usr/local/lib/web-app-framework && yarn install --frozen-lockfile --network-timeout 100000
RUN cd /usr/local/lib/web-app-framework && tar zcf node_modules.tgz node_modules
RUN rm -rf /usr/local/lib/web-app-framework/node_modules

RUN cd /usr/local/lib/webapp && yarn --network-timeout 100000 install --frozen-lockfile

# Adding in yalc, which helps manage web-app-framework dependency
# When we add packages to yalc, it gets copied 
# into .yalc folder and becomes globally accessible to other yarn repos.
# Need to have non root write access to this folder in order to allow web-app-framework updates.
RUN yarn config set prefix /root/.yarn
# adding yarn's globally installed package location to path
ENV PATH=/usr/local/go/bin:/opt/bin:/import/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/root/.yarn/bin
RUN yarn global add yalc

# Adding web-app-framework to yalc so that webapp's node modules have all dependencies installed
RUN mkdir -p /usr/local/lib/web-app-framework/
COPY web-app-framework/web-app-framework.tgz /usr/local/lib/web-app-framework/web-app-framework.tgz
RUN cd /usr/local/lib/web-app-framework/ && tar xvf web-app-framework.tgz && cd dist && yalc publish --private
RUN cd /usr/local/lib/webapp && yalc add web-app-framework
RUN cd /usr/local/lib/webapp && yarn install --offline

RUN cd /usr/local/lib/webapp && tar zcf node_modules.tgz node_modules
RUN rm -rf /usr/local/lib/webapp/node_modules

# clean yarn cache to reduce container size
RUN yarn cache clean

# Removing yalc link so that during container runtime, web-app-framework updates
# don't try to update the copy living in /usr/local/lib
RUN cd /usr/local/lib/webapp && yalc remove web-app-framework

RUN cp /usr/local/share/.yarnrc /root/

WORKDIR /import/src/github.com/pensando/sw
ENV HOME=/root
USER root
RUN chmod -R 777 /root
CMD make ui
