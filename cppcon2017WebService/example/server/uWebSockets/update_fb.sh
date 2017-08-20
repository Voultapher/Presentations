#!/usr/bin/env bash

# Temporary hack to generate flabuffer shema represenations

WS_DIR=$(readlink -f .)

cd $WS_DIR/include
${HOME}/Tools/flatbuffers/build/flatc --cpp $WS_DIR/include/strawpoll.fbs

cd $WS_DIR/../../client/strawpoll/src
${HOME}/Tools/flatbuffers/build/flatc --js $WS_DIR/include/strawpoll.fbs
