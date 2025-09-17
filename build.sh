#!/bin/bash

set -e

pushd app; ./build.sh; popd
pushd firmware; ./build.sh; popd