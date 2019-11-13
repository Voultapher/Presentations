#!/usr/bin/env bash

set -euxo pipefail

rm -f index.html
ln -s ../_style/index.html index.html

rm -f style
ln -s ../_style style

rm -f assets
ln -s "${1}" assets
