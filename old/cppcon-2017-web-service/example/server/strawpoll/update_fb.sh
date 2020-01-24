#!/usr/bin/env bash

set +e

SCRIPT_PATH=$(readlink -f "${0}")
SCRIPT_DIR=$(dirname "${SCRIPT_PATH}")

function build()
{
  # Path to your flatbuffers install
  ${HOME}/Tools/flatbuffers/build/flatc ${1} -o "${2}" \
  "${SCRIPT_DIR}/strawpoll.fbs"
}

build --cpp "${SCRIPT_DIR}"

JS_FOLDER=$(readlink -f "${SCRIPT_DIR}/../../client/strawpoll/src")
JS_FILE="${JS_FOLDER}/strawpoll_generated.js"

build --js "${JS_FOLDER}"
printf "/*eslint-disable */\n$(cat ${JS_FILE})" > ${JS_FILE}
