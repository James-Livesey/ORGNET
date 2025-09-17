#!/bin/bash

set -e

FILE=main.asm
BIN_FILE=${FILE%.*}.bin
OPK_FILE=${FILE%.*}.opk

toolchain/psion-org2-assembler/org2asm.tcl -f $FILE

python3 toolchain/makeopk.py $BIN_FILE $OPK_FILE

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
fi