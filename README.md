# ORGNET
A wireless networking peripheral for the Psion Organiser II, in a datapack form factor.

## Getting the code
This repository contains Git submodules that must be cloned to be able to assemble the ORGNET application's code and run an emulated Organiser II. To clone this repository, run:

```bash
git clone --recurse-submodules https://github.com/James-Livesey/ORGNET
```

## Building and running
To build ORGNET's firmware and application, run:

```bash
./build.sh
```

Running this script without any arguments will also upload the firmware and application build to the RP2350 over USB.

To test ORGNET's application in the emulator, run:

```bash
pushd app; ./build.sh --test; popd
```
