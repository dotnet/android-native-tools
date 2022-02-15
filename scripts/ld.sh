#!/bin/bash -e
MY_DIR="$(cd $(dirname $0);pwd)"
exec "${MY_DIR}"/ld "$@"
