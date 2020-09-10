FROM ubuntu:14.04

MAINTAINER samhrussell

RUN apt-get update
RUN apt-get install -y wget
RUN apt-get install -y xz-utils
RUN apt-get install -y build-essential
RUN apt-get install -y ccache
RUN apt-get install -y bc

ENV CCACHE_DIR=/ccache
ENV PATH=/usr/lib/ccache:$PATH

WORKDIR /linux

RUN wget https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.3.tar.xz
RUN tar -xf linux-4.3.tar.xz

WORKDIR linux-4.3

RUN make defconfig

RUN echo "CONFIG_MPLS=y" >> .config
RUN echo "CONFIG_NET_MPLS_GSO=m" >> .config
RUN echo "CONFIG_MPLS_ROUTING=m" >> .config
RUN echo "CONFIG_MPLS_IPTUNNEL=m" >> .config
RUN echo "CONFIG_LWTUNNEL=y" >> .config

RUN echo "ccache -M 10G" > /instructions.sh
RUN echo "make -j `getconf _NPROCESSORS_ONLN`" >> /instructions.sh
RUN echo "make -j `getconf _NPROCESSORS_ONLN` deb-pkg LOCALVERSION=-custom" >> /instructions.sh
RUN echo "cp ../*.deb /kernel/" >> /instructions.sh

CMD bash -c 'time sh /instructions.sh'
