#!/usr/bin/env bash

# The path to the folder you'd like to share (This needs to be the root directory of this git repo)
HOST_PATH=$1

# The name of the development image
DOCKER_IMAGE_NAME=development-docker

DOCKER_CONTAINER_NAME=dev-container

# Build it - this can take a bit, but should only happen once
docker build -t $DOCKER_IMAGE_NAME .

# Run the development docker with the details needed to use it for dev work.
# Requires: Xming, volume sharing
if [ ! "$(docker ps -q -f name=$DOCKER_CONTAINER_NAME)" ]; then
    if [ "$(docker ps -aq -f status=exited -f name=$DOCKER_CONTAINER_NAME)" ]; then
        # Use the old one
        echo "Found the container, exited, starting and attaching."
        winpty docker start -ai $DOCKER_CONTAINER_NAME
    fi

    # start a new one
    echo "Didnt find the container, spinning up a new one."
    winpty docker run \
        --name $DOCKER_CONTAINER_NAME  \
        --network host \
        -e DISPLAY=host.docker.internal:0 \
        -v $HOST_PATH:/home/user/hovergames2 \
        -it $DOCKER_IMAGE_NAME bash
else
    if [ "$(docker ps -aq -f status=running -f name=$DOCKER_CONTAINER_NAME)" ]; then
        # Use the old one
        echo "Found the container already running, using this one."
        winpty docker exec -it $DOCKER_CONTAINER_NAME bash
    fi
fi



# Hold up the terminal incase there is an error so it doesnt close without being able to see the errors
echo "Press any key to close"
while [ true ] ; do
read -t 3 -n 1
if [ $? = 0 ] ; then
exit ;
else
echo "-"
fi
done
