#!/bin/bash

set -e

FILE=main.asm
BIN_FILE=${FILE%.*}.bin
OPK_FILE=${FILE%.*}.opk

if [ "$1" = "--test" ]; then
    echo ".INCLUDE comms.mock.inc" > comms.inc
else
    echo ".INCLUDE comms.real.inc" > comms.inc
fi

toolchain/psion-org2-assembler/org2asm.tcl -f $FILE

echo ".INCLUDE comms.real.inc" > comms.inc

if [ ! -f toolchain/devkit/BLDPACK.EXE ]; then
    mkdir -p toolchain/devkit

    pushd toolchain/devkit
        wget https://www.jaapsch.net/psion/software/pc2/devkit.zip
        unzip devkit.zip
    popd
fi

mkdir -p toolchain/devkit/RES
cp toolchain/build.bat toolchain/devkit/RES/BUILD.BAT
cp main.bin toolchain/devkit/RES/MAIN.BIN
cp orgnet.bld toolchain/devkit/RES/ORGNET.BLD
cp ../app/* toolchain/devkit/RES/

dosbox -exit toolchain/devkit/RES/BUILD.BAT

cp toolchain/devkit/ORGNET.OPK $OPK_FILE

if [ "$1" = "--test" ]; then
    if [ ! -f toolchain/Psiora/psiora ]; then
        pushd toolchain/Psiora
            qmake psiora.pro
            make all
        popd
    fi

    if [ ! -f boot.rom ]; then
        wget https://www.jaapsch.net/psion/images/roms/31-xp.rom -O boot.rom
    fi

    toolchain/Psiora/psiora --rom-file boot.rom --pak-b-file $OPK_FILE --no-close-confirm
else
    # Ensure pack type is set to be RAM-based
    printf "\x6C" | dd of=$OPK_FILE bs=1 seek=6 count=1 conv=notrunc
fi