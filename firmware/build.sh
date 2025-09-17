#!/bin/bash

set -e

mkdir -p build/picotool-local

if [ ! -f ../app/main.bin ]; then
    echo "Application not assembled yet; please assemble it first using app/build.sh"
    exit 1
fi

xxd -n appCode -i ../app/main.bin > _app.h

pushd build
    if [ ! -f picotool-local/picotool ]; then
        pushd picotool-local
            export PICO_SDK_PATH=../../lib/pico-sdk
            cmake ../../lib/picotool
            make
        popd
    fi

    export PICO_SDK_PATH=../lib/pico-sdk
    cmake .. -DPICO_BOARD=pico2_w
    make orgnet
    sudo picotool-local/picotool load -v -x orgnet.uf2 -f
popd