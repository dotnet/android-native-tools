#!/bin/bash -e
MY_NAME="$(basename $0)"
MY_DIR="$(cd $(dirname $0);pwd)"

exec "${MY_DIR}"/as @gas-arch=${MY_NAME} "$@"
