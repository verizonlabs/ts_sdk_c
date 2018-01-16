# docker build -t toolchain toolchain/Dockerfile
FROM ubuntu:16.04

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
    && apt-get install -y perl

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

# download and setup toolchains
RUN mkdir /toolchain

# toolchain, arm-linux-gnueabihf (cortex-A, raspberrypi)
RUN wget https://github.com/raspberrypi/tools/archive/master.tar.gz \
    && tar -C /toolchain/ -xvzf master.tar.gz \
    && cp -R /toolchain/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64 /toolchain/arm-linux-gnueabihf \
    && rm -rf /toolchain/tools-master \
    && rm -f master.tar.gz

# toolchain, arm-none-eabi (cortex-M and cortex-R, bare-metal)
# (TODO - should change to, https://developer.arm.com/open-source/gnu-toolchain/gnu-rm)
RUN wget -O gcc-arm-none-eabi-5_4-2016q2-20160622-linux.tar.bz2 https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q2-update/+download/gcc-arm-none-eabi-5_4-2016q2-20160622-linux.tar.bz2 \
    && tar -C /toolchain/ -jxvf gcc-arm-none-eabi-5_4-2016q2-20160622-linux.tar.bz2 \
    && mv /toolchain/gcc-arm-none-eabi-5_4-2016q2 /toolchain/arm-none-eabi \
    && rm -f gcc-arm-none-eabi-5_4-2016q2-20160622-linux.tar.bz2

# toolchain, arm-eabi (cortext-A, bare-metal)
RUN wget -O gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz https://releases.linaro.org/components/toolchain/binaries/latest/arm-eabi/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz \
    && unxz gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz \
    && tar -C /toolchain/ -xvf gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar \
    && mv /toolchain/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi /toolchain/arm-eabi \
    && rm -f gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar

# update path
ENV PATH "/usr/local/bin:${PATH}"
ENV PATH "/toolchain/arm-linux-gnueabihf/bin:${PATH}"
ENV PATH "/toolchain/arm-none-eabi/bin:${PATH}"
ENV PATH "/toolchain/arm-eabi/bin:${PATH}"

# docker run -v $(pwd):/source -ti toolchain ./toolchain/cmake.bash
WORKDIR /source
