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
