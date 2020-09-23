FROM ubuntu:18.04

RUN apt update

# Install dev tools
RUN apt install -y gedit vim

# Install build tools
RUN apt install -y make cmake gcc g++ build-essential

# Create a user
RUN useradd -ms /bin/bash user
USER user
WORKDIR /home/user

# Setup folder for shared drive
RUN mkdir -p /home/user/hovergames2

# Create a clion executable alias
RUN echo "alias clion=\"/home/user/hovergames2/docker/clion-2020.2.3/bin/clion.sh\"" >> ~/.bashrc
