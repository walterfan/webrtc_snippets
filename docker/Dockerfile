FROM ubuntu:20.04

LABEL maintainer="Walter Fan <walter.fan@gmail.com>" \
      license="MIT"

WORKDIR /workspace

RUN echo "Installing required packages " \
         && export DEBIAN_FRONTEND=noninteractive  \
         && apt-get update \
         && apt-get install -y \
               vim \
               git \
               wget \
               gnupg2 \
               lsb-release \
               apt-utils \
               software-properties-common \
               build-essential \
               cmake \
               scons \
               libssl-dev \
               libgtest-dev \
               libgmock-dev \
               libjsoncpp-dev \
               libboost-all-dev \
         && apt-get autoremove --purge -y \
         && apt-get autoclean -y \
         && rm -rf /var/cache/apt/*

ADD install_deps.sh /install_deps.sh

RUN echo "Installing Dependencies" \
         && chmod +x /install_deps.sh \
         && sh /install_deps

ENTRYPOINT [ "/bin/bash" ]         