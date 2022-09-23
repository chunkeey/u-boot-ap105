FROM        debian:10

ARG         DEBIAN_FRONTEND=noninteractive
ARG         TOOLCHAIN_SRC=https://downloads.openwrt.org/releases/21.02.2/targets/ath79/generic/openwrt-sdk-21.02.2-ath79-generic_gcc-8.4.0_musl.Linux-x86_64.tar.xz

USER root

RUN echo 'deb http://deb.debian.org/debian testing main' \
		> /etc/apt/sources.list.d/testing.list && \
	apt-get update && \
	apt-get install -t buster -y \
		pwgen \
		locales \
		build-essential \
		git-core \
		subversion \
		libncurses5-dev \
		gawk \
        gcc-multilib \
		unzip \
		pv \
		gosu \
		signify-openbsd \
		python3 \
		wget \
		curl \
		screen \
		rsync && \
	apt-get clean && \
	localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8

ENV LANG=en_US.utf8



RUN mkdir /openwrt-sdk && \
	curl "${TOOLCHAIN_SRC}" | tar -xJf - --strip-components=1 -C /openwrt-sdk

# RUN git clone https://github.com/hurricos/u-boot-ap105 --branch ap175_vsc --depth 1 /u-boot-ap105
ADD ./ /u-boot-ap105/

ADD ./docker/compile.sh /compile.sh

RUN [ "/compile.sh" ]

