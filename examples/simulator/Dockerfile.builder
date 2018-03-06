# docker build -t simulator-builder -f Dockerfile.build .
FROM ubuntu:17.10

# setup ubuntu
RUN apt-get update \
    && apt-get install -y ntp \
    && apt-get install -y sudo \
    && apt-get install -y wget \
    && apt-get install -y build-essential \
    && apt-get install -y gcc-multilib \
    && apt-get install -y xz-utils \
    && apt-get install -y bzip2 \
    && apt-get install -y vim \
    && apt-get install -y perl \
    && apt-get install -y git

# build cmake
RUN wget https://cmake.org/files/v3.9/cmake-3.9.6.tar.gz \
    && tar -xzvf cmake-3.9.6.tar.gz \
    && cd cmake-3.9.6 \
    && ./bootstrap \
    && make \
    && make install \
    && cd .. \
    && rm -rf cmake-3.9.6 \
    && rm -f cmake-3.9.6.tar.gz

# update path
ENV PATH "/usr/local/bin:${PATH}"

WORKDIR /source

