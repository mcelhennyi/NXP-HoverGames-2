FROM ubuntu:18.04

RUN apt update

# Install dev tools
RUN apt install -y gedit vim

# Install build tools
RUN apt install -y make cmake gcc g++ build-essential

ENV TZ=America/Chicago
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install python deps
RUN apt update && apt install curl git python3-pip idle3 -y && \
  pip3 install --no-cache-dir --upgrade pip && \
  \
  # make some useful symlinks that are expected to exist
  cd /usr/bin && \
  ln -s pydoc3 pydoc && \
  ln -s python3 python && \
  ln -s python3-config python-config && \
  cd /

RUN apt install -y castxml

RUN apt update && apt install -y  --fix-missing \
    software-properties-common \
    autoconf \
    automake \
    autotools-dev \
    ca-certificates \
    clang-format \
    colordiff \
    doxygen \
    libcurl4-openssl-dev \
    libltdl-dev \
    libtinyxml2-dev \
    libtool \
    ruby-dev \
    libz-dev \
    golang-go \
    sudo

# delete cache and tmp files
RUN apt clean && \
  apt autoclean && \
  rm -rf /var/cache/* && \
  rm -rf /tmp/* && \
  rm -rf /var/tmp/* && \
  rm -rf /var/lib/apt/lists/*

RUN gem install --no-ri --no-rdoc fpm;

# ADD /sudoers.txt /etc/sudoers

# Create a user
RUN useradd -ms /bin/bash user && echo "user:pass" | chpasswd
RUN chmod 440 /etc/sudoers
RUN  adduser user sudo
USER user
WORKDIR /home/user

# Setup folder for shared drive
RUN mkdir -p /home/user/hovergames2

# Create a clion executable alias
RUN echo "alias clion=\"/home/user/hovergames2/docker/clion-2020.2.3/bin/clion.sh\"" >> ~/.bashrc
RUN echo "~/hovergames2/setup.sh" >> ~/.bashrc

# PX4 SITL Requirements

# Make sure python works properly (might need to map python to python3 as well?) - https://github.com/PX4/PX4-Autopilot/issues/16261
ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8

# Setup the code repo - https://docs.px4.io/master/en/simulation/
RUN git clone https://github.com/PX4/PX4-Autopilot.git ~/px4 --recursive
RUN cd ~/px4 && bash ./Tools/setup/ubuntu.sh

# Setup mavlink Router
RUN cd ~/ && git clone https://github.com/mavlink-router/mavlink-router.git
RUN cd ~/mavlink-router && git submodule update --init --recursive

USER root
RUN apt update && apt install -y python-future python3-future libtool autoconf
USER user

USER root
RUN echo "user ALL=(ALL:ALL) NOPASSWD: ALL" | sudo tee /etc/sudoers.d/dont-prompt-user-for-sudo-password
USER user

RUN cd ~/mavlink-router && ./autogen.sh && ./configure CFLAGS='-g -O2' \
    --sysconfdir=/etc --disable-systemd --localstatedir=/var --libdir=/usr/lib64 \
    --prefix=/usr
RUN cd ~/mavlink-router &&  make
RUN cd ~/mavlink-router &&  sudo make install

# MAVSDK
RUN mkdir -p ~/thirdparty && \
    cd ~/thirdparty && \
    git clone https://github.com/mavlink/MAVSDK.git mavsdk && \
    cd mavsdk && \
    git checkout v0.35.1 && \
    git submodule init && \
    git submodule update --recursive --init

RUN sudo apt-get install -y net-tools

EXPOSE 5000-15000
RUN cd ~/px4 && sudo bash ./Tools/setup/ubuntu.sh

# SITL setup for quad with optical flow - https://docs.px4.io/master/en/simulation/gazebo.html
# RUN HEADLESS=1 make px4_sitl gazebo_iris_opt_flow
# RUN mavlink-routerd -e <QGC PATH>:14550 127.0.0.1:14550
