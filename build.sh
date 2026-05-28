#!/bin/bash

set -e

pushd service; ./build.sh; popd
pushd firmware; ./build.sh; popd