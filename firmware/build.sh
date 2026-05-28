#!/bin/bash

set -e

mkdir -p build/picotool-local

if [ ! -f ../service/main.bin ]; then
    echo "Service not assembled yet; please assemble it first using service/build.sh"
    exit 1
fi

xxd -n serviceCode -i ../service/main.bin > _service.h

pushd build
    if [ ! -f picotool-local/picotool ]; then
        pushd picotool-local
            export PICO_SDK_PATH=../../lib/pico-sdk
            cmake ../../lib/picotool
            make
        popd
    fi

    export PICO_SDK_PATH=../lib/pico-sdk
    export ORGNET_BOARD_DEST=../lib/pico-sdk/src/boards/include/boards/orgnet.h

    if ! [ -f $ORGNET_BOARD_DEST ]; then
        cp ../board.h ../lib/pico-sdk/src/boards/include/boards/orgnet.h
    fi

    cmake .. -DPICO_BOARD=orgnet
    make orgnet
    # sudo picotool-local/picotool load -v -x orgnet.uf2 -f
popd