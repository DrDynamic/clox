#!/bin/bash

PROJECT_PATH=`realpath $(dirname "$0")/..`
REBUILD_IMAGE=false
DOCKER_USER=`id -u`:`id -g`
GCC_ARGS=""

# check input args
while [ $# -gt 0 ]; do
  case "$1" in
    --build-image)
      REBUILD_IMAGE=true
      ;;
    *)
      GCC_ARGS="$GCC_ARGS $1"
  esac
  shift
done

if [ -z "$(docker images -q clox/builder 2> /dev/null)" ] || [ "$REBUILD_IMAGE" = true ]; then
    # image doexnt exist -> build it 
    docker build $PROJECT_PATH/docker -t clox/builder
fi

docker run --rm --user $DOCKER_USER -v $PROJECT_PATH:/usr/src/clox -w /usr/src/clox clox/builder gcc $GCC_ARGS